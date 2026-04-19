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
    return res.status(400).json({ error: 'Invalid code input' })
  }

  const timestamp = Date.now()
  const tempFile = path.join(TEMP_DIR, `input_${timestamp}.txt`)
  const astFile = path.join(COMPILER_PATH, 'ast.json')  // Main outputs to root
  const annotatedAstFile = path.join(TEMP_DIR, 'annotated_ast.json')  // semantic_main uses fixed names
  const symbolTableFile = path.join(TEMP_DIR, 'symbol_table.json')
  const irFile = path.join(TEMP_DIR, 'ir.txt')

  try {
    // Write the source code to a temporary file
    await fs.writeFile(tempFile, code, 'utf-8')

    // Step 1: Run the main compiler (lexer + parser)
    console.log('Step 1: Running lexer and parser...')
    
    // Run Main.exe with file argument
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
    
    await new Promise((resolve, reject) => {
      mainProcess.on('close', (code) => {
        if (code !== 0 && code !== -1) {
          reject(new Error(`Main.exe exited with code ${code}`))
        } else {
          resolve()
        }
      })
      mainProcess.on('error', reject)
    })

    console.log('Main output:', mainOutput)
    if (mainError) {
      console.error('Parser stderr:', mainError)
    }

    // ast.json is already in COMPILER_PATH, no need to copy

    // Step 2: Run semantic analyzer
    console.log('Step 2: Running semantic analyzer...')
    
    const semanticProcess = spawn(
      path.join(COMPILER_PATH, 'semantic', 'semantic_main.exe'),
      [astFile, TEMP_DIR],
      { timeout: 10000 }
    )
    
    let semanticOutput = ''
    let semanticError = ''
    
    semanticProcess.stdout.on('data', (data) => {
      semanticOutput += data.toString()
    })
    
    semanticProcess.stderr.on('data', (data) => {
      semanticError += data.toString()
    })
    
    await new Promise((resolve, reject) => {
      semanticProcess.on('close', (code) => {
        if (code !== 0 && code !== -1) {
          reject(new Error(`Semantic analyzer exited with code ${code}`))
        } else {
          resolve()
        }
      })
      semanticProcess.on('error', reject)
    })

    console.log('Semantic output:', semanticOutput)
    if (semanticError) {
      console.error('Semantic stderr:', semanticError)
    }

    // Step 3: Run code generator
    console.log('Step 3: Running code generator...')
    
    const codegenProcess = spawn(
      path.join(COMPILER_PATH, 'codegen', 'codegen.exe'),
      [annotatedAstFile, symbolTableFile, irFile],
      { timeout: 10000 }
    )
    
    let codegenOutput = ''
    let codegenError = ''
    
    codegenProcess.stdout.on('data', (data) => {
      codegenOutput += data.toString()
    })
    
    codegenProcess.stderr.on('data', (data) => {
      codegenError += data.toString()
    })
    
    await new Promise((resolve, reject) => {
      codegenProcess.on('close', (code) => {
        if (code !== 0 && code !== -1) {
          reject(new Error(`Code generator exited with code ${code}`))
        } else {
          resolve()
        }
      })
      codegenProcess.on('error', reject)
    })

    console.log('Codegen output:', codegenOutput)
    if (codegenError) {
      console.error('Codegen stderr:', codegenError)
    }

    if (codegenError) {
      console.error('Codegen stderr:', codegenError)
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

    // Return the results
    res.json({
      ast: ast || annotatedAst,  // Use original AST if available, fallback to annotated
      symbolTable: symbolTable,
      tac: tac
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

    res.status(500).json({
      error: error.message || 'Compilation failed',
      details: error.stderr || error.stdout || ''
    })
  }
})

app.listen(PORT, () => {
  console.log(`🚀 Compiler API server running on http://localhost:${PORT}`)
  console.log(`📁 Compiler path: ${COMPILER_PATH}`)
  console.log(`📂 Temp directory: ${TEMP_DIR}`)
})
