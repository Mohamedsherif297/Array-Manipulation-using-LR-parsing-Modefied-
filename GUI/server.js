import express from 'express'
import cors from 'cors'
import { exec, spawn } from 'child_process'
import { promisify } from 'util'
import fs from 'fs/promises'
import path from 'path'
import { fileURLToPath } from 'url'

const execAsync = promisify(exec)
const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

const app = express()
const PORT = 3003

app.use(cors())
app.use(express.json())

// Path to the compiler project
const COMPILER_PATH = path.join(__dirname, '..', 'Array-Manipulation-using-LR-parsing--Karim-Ahmed')

// Temporary directory for compilation
const TEMP_DIR = path.join(__dirname, 'temp')

// Ensure temp directory exists
await fs.mkdir(TEMP_DIR, { recursive: true })

app.post('/api/compile', async (req, res) => {
  const { code } = req.body

  if (!code || typeof code !== 'string') {
    return res.status(400).json({ 
      success: false,
      errors: [{ message: 'Invalid code input', phase: 'unknown' }]
    })
  }

  const timestamp = Date.now()
  const tempFile = path.join(TEMP_DIR, `input_${timestamp}.txt`)
  const astFile = path.join(COMPILER_PATH, 'ast.json')
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
      const [ast, annotatedAst, symbolTable] = await Promise.all([
        fs.readFile(astFile, 'utf-8').then(JSON.parse).catch(() => null),
        fs.readFile(annotatedAstFile, 'utf-8').then(JSON.parse).catch(() => null),
        fs.readFile(symbolTableFile, 'utf-8').then(JSON.parse).catch(() => null)
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
        tac: null
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
    const [ast, annotatedAst, symbolTable, tac] = await Promise.all([
      fs.readFile(astFile, 'utf-8').then(JSON.parse).catch(() => null),
      fs.readFile(annotatedAstFile, 'utf-8').then(JSON.parse).catch(() => null),
      fs.readFile(symbolTableFile, 'utf-8').then(JSON.parse).catch(() => null),
      fs.readFile(irFile, 'utf-8').catch(() => null)
    ])

    // Clean up temporary files
    await Promise.all([
      fs.unlink(tempFile).catch(() => {}),
      fs.unlink(astFile).catch(() => {}),
      fs.unlink(annotatedAstFile).catch(() => {}),
      fs.unlink(symbolTableFile).catch(() => {}),
      fs.unlink(irFile).catch(() => {})
    ])

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
      tac
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
