import express from 'express'
import cors from 'cors'
import { spawn } from 'child_process'
import fs from 'fs/promises'
import path from 'path'
import { fileURLToPath } from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

const app = express()
const PORT = 3003

app.use(cors())
app.use(express.json())

// Path to the compiler project — one level up from GUI/ is the project root
const COMPILER_PATH = path.join(__dirname, '..')

// Temporary directory for compilation
const TEMP_DIR = path.join(__dirname, 'temp')

// Ensure temp directory exists
await fs.mkdir(TEMP_DIR, { recursive: true })

function defaultValueForType(type) {
  switch (type) {
    case 'char': return ''
    case 'string': return ''
    case 'int':
    case 'float':
    case 'double':
    default: return 0
  }
}

function castValueToType(value, type) {
  switch (type) {
    case 'char': {
      if (typeof value === 'number') return String.fromCharCode(value)
      const s = String(value ?? '')
      return s.length > 0 ? s[0] : ''
    }
    case 'string':
      return String(value ?? '')
    case 'float':
    case 'double':
    case 'int': {
      const n = Number(value)
      return Number.isFinite(n) ? n : 0
    }
    default:
      return value
  }
}

// Count how many cin >> targets exist in the AST
function countCinTargets(node) {
  if (!node) return 0
  if (node.type === 'Input') return (node.children || []).length
  return (node.children || []).reduce((sum, c) => sum + countCinTargets(c), 0)
}

function buildInitialRuntimeState(symbolTable) {
  const env = {}
  if (!symbolTable || typeof symbolTable !== 'object') return env

  for (const [name, sym] of Object.entries(symbolTable)) {
    const type = sym?.type || 'int'
    if (sym?.isArray) {
      const size1 = Number(sym.size1 || 0)
      const size2 = Number(sym.size2 || 0)
      if (size2 > 0) {
        env[name] = Array.from({ length: size1 }, () =>
          Array.from({ length: size2 }, () => defaultValueForType(type))
        )
      } else {
        env[name] = Array.from({ length: size1 }, () => defaultValueForType(type))
      }
    } else {
      env[name] = defaultValueForType(type)
    }
  }

  return env
}

function collectArrayAccessInfo(node) {
  const indices = []
  let current = node

  while (current && current.type === 'ArrayAccess') {
    if (!current.children || current.children.length < 2) break
    indices.push(current.children[1])
    current = current.children[0]
  }

  if (!current || current.type !== 'ID') return null
  indices.reverse()
  return { baseName: current.value, indexNodes: indices }
}

function runProgram(ast, symbolTable, stdinText = '') {
  const env = buildInitialRuntimeState(symbolTable)
  const inputTokens = String(stdinText || '').split(/\s+/).filter(Boolean)
  let inputPos = 0
  let output = ''
  const runtimeWarnings = []

  const getSymbolType = (name) => symbolTable?.[name]?.type || 'int'

  const evalExpr = (node) => {
    if (!node) return 0
    const t = node.type

    if (t === 'NUM' || t === 'Number') {
      return Number(node.value)
    }
    if (t === 'STRING') return String(node.value ?? '')
    if (t === 'CHAR') return String(node.value ?? '').slice(0, 1)
    if (t === 'EndLine') return '\n'
    if (t === 'ID') return env[node.value]

    if (t === 'ArrayAccess') {
      const info = collectArrayAccessInfo(node)
      if (!info) return 0
      const base = env[info.baseName]
      const indices = info.indexNodes.map((n) => Number(evalExpr(n)))

      if (typeof base === 'string') {
        const idx = indices[0] ?? 0
        return idx >= 0 && idx < base.length ? base[idx] : ''
      }

      if (!Array.isArray(base)) return 0
      if (indices.length === 1) return base[indices[0]]
      if (indices.length >= 2 && Array.isArray(base[indices[0]])) return base[indices[0]][indices[1]]
      return 0
    }

    if (t === '+' || t === '-' || t === '*' || t === '/') {
      const l = evalExpr(node.children?.[0])
      const r = evalExpr(node.children?.[1])
      if (t === '+') return l + r
      if (t === '-') return Number(l) - Number(r)
      if (t === '*') return Number(l) * Number(r)
      if (t === '/') return Number(r) === 0 ? 0 : Number(l) / Number(r)
    }

    if (Array.isArray(node.children) && node.children.length > 0) {
      return evalExpr(node.children[node.children.length - 1])
    }

    return 0
  }

  const assignTarget = (lhsNode, rhsValue) => {
    if (!lhsNode) return

    if (lhsNode.type === 'ID') {
      const name = lhsNode.value
      env[name] = castValueToType(rhsValue, getSymbolType(name))
      return
    }

    if (lhsNode.type !== 'ArrayAccess') return

    const info = collectArrayAccessInfo(lhsNode)
    if (!info) return
    const baseName = info.baseName
    const indices = info.indexNodes.map((n) => Number(evalExpr(n)))
    const current = env[baseName]

    if (typeof current === 'string') {
      const idx = indices[0] ?? 0
      if (idx < 0) return

      const rhsString = String(rhsValue ?? '')
      // Requested behavior: allow editing string with string value at index.
      // Example: name[1] = "are";
      env[baseName] =
        current.slice(0, idx) +
        rhsString +
        current.slice(Math.min(idx + 1, current.length))
      return
    }

    if (!Array.isArray(current)) return

    if (indices.length === 1) {
      current[indices[0]] = castValueToType(rhsValue, getSymbolType(baseName))
      return
    }

    if (indices.length >= 2 && Array.isArray(current[indices[0]])) {
      current[indices[0]][indices[1]] = castValueToType(rhsValue, getSymbolType(baseName))
    }
  }

  const readInputForTarget = (targetNode) => {
    const raw = inputTokens[inputPos++]
    if (raw == null) {
      // cin called but no input provided — warn and use default
      const varName = targetNode?.type === 'ID'
        ? targetNode.value
        : (targetNode?.type === 'ArrayAccess'
            ? collectArrayAccessInfo(targetNode)?.baseName
            : '?') ?? '?'
      runtimeWarnings.push(
        `cin >> ${varName}: no input value provided. ` +
        `Enter values in the Console Input box before compiling.`
      )
      return ''
    }

    if (targetNode?.type === 'ID') {
      const type = getSymbolType(targetNode.value)
      return castValueToType(raw, type)
    }

    if (targetNode?.type === 'ArrayAccess') {
      const info = collectArrayAccessInfo(targetNode)
      if (info) {
        const type = getSymbolType(info.baseName)
        return castValueToType(raw, type)
      }
    }

    return raw
  }

  const execute = (node) => {
    if (!node) return false

    // ── Program: run globals first, then main ──────────────────────────────
    if (node.type === 'Program') {
      const globals = (node.children || []).filter(c => c.type !== 'FunctionDef')
      const mainFn  = (node.children || []).find(c => c.type === 'FunctionDef')

      // Phase 1: execute all global declarations/assignments
      for (const g of globals) {
        execute(g)
      }

      // Phase 2: execute main()
      if (mainFn) {
        const body = mainFn.children?.[2]
        return execute(body)
      }
      return false
    }

    if (node.type === 'StmtList') {
      for (const child of node.children || []) {
        if (execute(child)) return true
      }
      return false
    }

    // FunctionDef reached directly (no globals case — Program → FunctionDef)
    if (node.type === 'FunctionDef') {
      const body = node.children?.[2]
      return execute(body)
    }

    if (node.type === 'Declaration') {
      const name = node.children?.[1]?.value
      if (name && env[name] == null) {
        env[name] = defaultValueForType(node.children?.[0]?.value)
      }
      return false
    }

    if (node.type === 'DeclAssign') {
      const lhs = node.children?.[1]
      const rhs = node.children?.[node.children.length - 1]
      if (rhs?.type === 'ArrayInit') {
        const name = lhs?.value
        if (name) {
          const current = env[name]
          const elems = rhs.children || []
          if (Array.isArray(current) && current.length > 0 && Array.isArray(current[0])) {
            // 2D array: flatten initializers row by row
            let idx = 0
            for (let r = 0; r < current.length; r++) {
              for (let c = 0; c < current[r].length; c++) {
                if (idx < elems.length) {
                  // each elem may itself be an ArrayInit row
                  const rowElem = elems[idx]
                  if (rowElem?.type === 'ArrayInit') {
                    const rowElems = rowElem.children || []
                    for (let k = 0; k < current[r].length; k++) {
                      current[r][k] = castValueToType(evalExpr(rowElems[k]), getSymbolType(name))
                    }
                  } else {
                    current[r][c] = castValueToType(evalExpr(rowElem), getSymbolType(name))
                    idx++
                    continue
                  }
                }
                idx++
              }
            }
          } else if (Array.isArray(current)) {
            // 1D array: assign each initializer value by index
            for (let i = 0; i < elems.length && i < current.length; i++) {
              current[i] = castValueToType(evalExpr(elems[i]), getSymbolType(name))
            }
          }
        }
        return false
      }
      assignTarget(lhs, evalExpr(rhs))
      return false
    }

    if (node.type === 'Assignment') {
      assignTarget(node.children?.[0], evalExpr(node.children?.[1]))
      return false
    }

    if (node.type === 'Output') {
      for (const expr of node.children || []) {
        if (expr.type === 'EndLine') {
          output += '\n'
        } else {
          output += String(evalExpr(expr))
        }
      }
      // Only add automatic newline if the statement had no endl
      const hasEndl = (node.children || []).some(c => c.type === 'EndLine')
      if (!hasEndl) output += '\n'
      return false
    }

    if (node.type === 'Input') {
      for (const target of node.children || []) {
        assignTarget(target, readInputForTarget(target))
      }
      return false
    }

    if (node.type === 'Return') {
      return true
    }

    for (const child of node.children || []) {
      if (execute(child)) return true
    }
    return false
  }

  execute(ast)
  return { output, warnings: runtimeWarnings }
}

app.post('/api/compile', async (req, res) => {
  const { code, stdin } = req.body

  if (!code || typeof code !== 'string') {
    return res.status(400).json({ 
      success: false,
      errors: [{ message: 'Invalid code input', phase: 'unknown' }]
    })
  }

  const timestamp = Date.now()
  const tempFile = path.join(TEMP_DIR, `input_${timestamp}.txt`)
  const astFile = path.join(COMPILER_PATH, 'ast.json')
  const traceFile = path.join(COMPILER_PATH, 'parse_trace.json')
  const annotatedAstFile = path.join(TEMP_DIR, 'annotated_ast.json')
  const symbolTableFile = path.join(TEMP_DIR, 'symbol_table.json')
  const irFile = path.join(TEMP_DIR, 'ir.txt')

  const phases = {
    lexical: false,
    syntax: false,
    semantic: false,
    codegen: false
  }

  const errors = []
  const warnings = []

  try {
    // Write the source code to a temporary file
    await fs.writeFile(tempFile, code, 'utf-8')

    // Step 1: Run the main compiler (lexer + parser)
    console.log('Step 1: Running lexer and parser...')
    
    const mainProcess = spawn(
      path.join(COMPILER_PATH, 'Main.exe'),
      [tempFile],
      { cwd: COMPILER_PATH, timeout: 10000 }
    )
    
    let mainOutput = ''
    let mainError = ''
    
    mainProcess.stdout.on('data', (data) => {
      mainOutput += data.toString()
    })
    
    mainProcess.stderr.on('data', (data) => {
      mainError += data.toString()
    })
    
    const mainExitCode = await new Promise((resolve) => {
      mainProcess.on('close', (code) => resolve(code))
      mainProcess.on('error', () => resolve(-1))
    })

    console.log('Main output:', mainOutput)
    console.log('Main error:', mainError)
    
    // Parse main output for phase status
    if (mainOutput.includes('Tokenization complete')) {
      phases.lexical = true
    }
    if (mainOutput.includes('Parsing Successful') || mainOutput.includes('Parsing successful')) {
      phases.syntax = true
    }
    
    // Check for syntax errors with better parsing
    if (mainOutput.includes('SYNTAX ERROR') || mainOutput.includes('Parsing failed')) {
      console.log('Detected syntax error, parsing output...')
      // Try multiple patterns to extract line numbers and error messages
      const lines = mainOutput.split('\n')
      
      let currentErrorLine = null
      let currentErrorMessage = ''
      
      for (let i = 0; i < lines.length; i++) {
        const line = lines[i]
        
        // Pattern 1: "Error at line X"
        const pattern1 = line.match(/Error at line\s+(\d+)/i)
        if (pattern1) {
          currentErrorLine = parseInt(pattern1[1])
          console.log(`Found error at line ${currentErrorLine}`)
          continue
        }
        
        // Pattern 2: "Line X: error message"
        const pattern2 = line.match(/Line\s+(\d+):\s*(.+)/i)
        if (pattern2) {
          console.log(`Found error at line ${pattern2[1]}: ${pattern2[2]}`)
          errors.push({
            message: pattern2[2].trim() || 'Syntax error detected',
            line: parseInt(pattern2[1]),
            phase: 'syntax'
          })
          currentErrorLine = null
          continue
        }
        
        // Pattern 3: Error message on next line after "Error at line X"
        if (currentErrorLine && (line.includes('Missing semicolon') || line.includes('Unexpected token'))) {
          console.log(`Found error message for line ${currentErrorLine}: ${line}`)
          errors.push({
            message: line.trim(),
            line: currentErrorLine,
            phase: 'syntax'
          })
          currentErrorLine = null
          continue
        }
        
        // Pattern 4: Just "SYNTAX ERROR" without line number
        if (line.includes('SYNTAX ERROR') && !errors.some(e => e.phase === 'syntax')) {
          console.log(`Found syntax error marker: ${line}`)
        }
      }
      
      // If no errors were parsed but we know there's a syntax error
      if (errors.length === 0) {
        console.log('No specific errors parsed, adding generic syntax error')
        errors.push({
          message: 'Syntax error in code',
          phase: 'syntax'
        })
      }
    }
    
    console.log('Parsed errors:', errors)

    // If syntax failed, stop here
    if (!phases.syntax) {
      return res.json({
        success: false,
        phases,
        errors,
        warnings
      })
    }

    // Step 2: Run semantic analyzer
    console.log('Step 2: Running semantic analyzer...')
    
    const semanticProcess = spawn(
      path.join(COMPILER_PATH, 'semantic', 'semantic_main.exe'),
      [astFile, TEMP_DIR],
      { timeout: 10000 }
    )
    
    let semanticOutput = ''
    
    semanticProcess.stdout.on('data', (data) => {
      semanticOutput += data.toString()
    })
    
    const semanticExitCode = await new Promise((resolve) => {
      semanticProcess.on('close', (code) => resolve(code))
      semanticProcess.on('error', () => resolve(-1))
    })

    console.log('Semantic output:', semanticOutput)
    
    // Parse semantic errors with better pattern matching
    const lines = semanticOutput.split('\n')
    
    for (const line of lines) {
      // Skip empty lines and non-error lines
      if (!line.trim() || !line.includes('-')) continue
      
      // Pattern 1: "  - error message (line X)"
      const pattern1 = line.match(/^\s*-\s*(.+?)\s*\(line\s+(\d+)\)/i)
      if (pattern1) {
        const errorMsg = pattern1[1].trim()
        const lineNum = parseInt(pattern1[2])
        
        console.log(`Found semantic error at line ${lineNum}: ${errorMsg}`)
        errors.push({
          message: errorMsg,
          line: lineNum,
          phase: 'semantic'
        })
        continue
      }
      
      // Pattern 2: "  - error message" (without line number)
      const pattern2 = line.match(/^\s*-\s*(.+)$/i)
      if (pattern2) {
        const errorMsg = pattern2[1].trim()
        
        // Try to extract line number from the message itself
        const lineMatch = errorMsg.match(/line\s+(\d+)/i)
        const lineNum = lineMatch ? parseInt(lineMatch[1]) : undefined
        
        console.log(`Found semantic error: ${errorMsg}`)
        errors.push({
          message: errorMsg,
          line: lineNum,
          phase: 'semantic'
        })
        continue
      }
      
      // Pattern 3: [Semantic Error] message (line X)
      const pattern3 = line.match(/\[Semantic Error\]\s*(.+?)(?:\s*\(.*?line[:\s]+(\d+).*?\))?/i)
      if (pattern3) {
        const errorMsg = pattern3[1].trim()
        const lineNum = pattern3[2] ? parseInt(pattern3[2]) : undefined
        
        console.log(`Found semantic error: ${errorMsg}`)
        errors.push({
          message: errorMsg,
          line: lineNum,
          phase: 'semantic'
        })
        continue
      }
    }
    
    // Check if semantic analysis passed
    if (semanticOutput.includes('No errors found') || (semanticExitCode === 0 && errors.filter(e => e.phase === 'semantic').length === 0)) {
      phases.semantic = true
    }

    console.log(`Semantic phase complete. Errors so far: ${errors.length}`)
    console.log('All errors collected:', JSON.stringify(errors, null, 2))
    
    // Only continue to code generation if there are NO errors so far
    // Code generation requires a valid, error-free AST
    if (errors.length > 0) {
      console.log('Skipping code generation due to previous errors')
      
      // Read available output files
      const [ast, annotatedAst, symbolTable, earlyTrace] = await Promise.all([
        fs.readFile(astFile, 'utf-8').then(JSON.parse).catch(() => null),
        fs.readFile(annotatedAstFile, 'utf-8').then(JSON.parse).catch(() => null),
        fs.readFile(symbolTableFile, 'utf-8').then(JSON.parse).catch(() => null),
        fs.readFile(traceFile, 'utf-8').then(JSON.parse).catch(() => null)
      ])

      // Clean up temporary files
      await Promise.all([
        fs.unlink(tempFile).catch(() => {}),
        fs.unlink(astFile).catch(() => {}),
        fs.unlink(annotatedAstFile).catch(() => {}),
        fs.unlink(symbolTableFile).catch(() => {})
      ])

      console.log('Final errors being sent to client:', JSON.stringify(errors, null, 2))

      // Return result with all collected errors
      return res.json({
        success: false,
        phases,
        errors,
        warnings,
        ast: ast || annotatedAst,
        symbolTable,
        tac: null,
        parseTrace: earlyTrace
      })
    }

    // Step 3: Run code generator
    console.log('Step 3: Running code generator...')
    
    const codegenProcess = spawn(
      path.join(COMPILER_PATH, 'codegen', 'codegen.exe'),
      [annotatedAstFile, symbolTableFile, irFile],
      { timeout: 10000 }
    )
    
    let codegenOutput = ''
    
    codegenProcess.stdout.on('data', (data) => {
      codegenOutput += data.toString()
    })
    
    const codegenExitCode = await new Promise((resolve) => {
      codegenProcess.on('close', (code) => resolve(code))
      codegenProcess.on('error', () => resolve(-1))
    })

    console.log('Codegen output:', codegenOutput)
    
    // Check if codegen passed
    if (codegenOutput.includes('Code generation successful') || codegenExitCode === 0) {
      phases.codegen = true
    }

    // Read all output files
    const [ast, annotatedAst, symbolTable, tac, parseTraceData] = await Promise.all([
      fs.readFile(astFile, 'utf-8').then(JSON.parse).catch(() => null),
      fs.readFile(annotatedAstFile, 'utf-8').then(JSON.parse).catch(() => null),
      fs.readFile(symbolTableFile, 'utf-8').then(JSON.parse).catch(() => null),
      fs.readFile(irFile, 'utf-8').catch(() => null),
      fs.readFile(traceFile, 'utf-8').then(JSON.parse).catch(() => null)
    ])

    // Clean up temporary files
    await Promise.all([
      fs.unlink(tempFile).catch(() => {}),
      fs.unlink(astFile).catch(() => {}),
      fs.unlink(annotatedAstFile).catch(() => {}),
      fs.unlink(symbolTableFile).catch(() => {}),
      fs.unlink(irFile).catch(() => {})
    ])

    let consoleOutput = ''
    let runtimeError = null

    // Optional lightweight runtime simulation for cout/cin.
    if (errors.length === 0 && phases.codegen && (ast || annotatedAst)) {
      try {
        const astToRun = ast || annotatedAst
        const requiredInputs = countCinTargets(astToRun)
        const providedInputs = (stdin || '').trim().split(/\s+/).filter(Boolean).length

        const runtime = runProgram(astToRun, symbolTable, stdin)
        consoleOutput = runtime.output
        warnings.push(...runtime.warnings)

        // Inform the UI if cin needs input that wasn't provided
        if (requiredInputs > 0 && providedInputs < requiredInputs) {
          warnings.unshift(
            `This program uses cin (${requiredInputs} input value${requiredInputs > 1 ? 's' : ''} expected). ` +
            `Enter values in the Console Input box and compile again.`
          )
        }
      } catch (runtimeErr) {
        runtimeError = `Runtime simulation failed: ${runtimeErr.message || runtimeErr}`
        warnings.push(runtimeError)
      }
    }

    // Determine overall success - only successful if no errors in any phase
    const overallSuccess = errors.length === 0 && phases.codegen
    
    console.log(`Compilation complete. Success: ${overallSuccess}, Total errors: ${errors.length}`)
    console.log('Final errors being sent to client:', JSON.stringify(errors, null, 2))

    // Return result with all collected errors
    res.json({
      success: overallSuccess,
      phases,
      errors,
      warnings,
      ast: ast || annotatedAst,
      symbolTable,
      tac,
      consoleOutput,
      runtimeError,
      parseTrace: parseTraceData
    })

  } catch (error) {
    console.error('Compilation error:', error)
    
    // Clean up on error
    await Promise.all([
      fs.unlink(tempFile).catch(() => {}),
      fs.unlink(astFile).catch(() => {}),
      fs.unlink(annotatedAstFile).catch(() => {}),
      fs.unlink(symbolTableFile).catch(() => {}),
      fs.unlink(irFile).catch(() => {})
    ])

    res.json({
      success: false,
      phases,
      errors: [{ message: error.message || 'Compilation failed', phase: 'unknown' }],
      warnings
    })
  }
})

app.listen(PORT, () => {
  console.log(`🚀 Compiler API server running on http://localhost:${PORT}`)
  console.log(`📁 Compiler path: ${COMPILER_PATH}`)
  console.log(`📂 Temp directory: ${TEMP_DIR}`)
})
