import express from 'express'
import cors from 'cors'
import { exec } from 'child_process'
import { promisify } from 'util'
import fs from 'fs/promises'
import path from 'path'
import { fileURLToPath } from 'url'

const execAsync = promisify(exec)
const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

const app = express()
const PORT = 3001

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
  const astFile = path.join(COMPILER_PATH, 'ast.json')  // Main outputs to its own directory
  const annotatedAstFile = path.join(TEMP_DIR, 'annotated_ast.json')  // semantic_main uses fixed names
  const symbolTableFile = path.join(TEMP_DIR, 'symbol_table.json')
  const irFile = path.join(TEMP_DIR, 'ir.txt')

  try {
    // Write the source code to a temporary file
    await fs.writeFile(tempFile, code, 'utf-8')

    // Step 1: Run the main compiler (lexer + parser)
    console.log('Step 1: Running lexer and parser...')
    const mainExe = path.join(COMPILER_PATH, 'Main')
    
    // Modify Main.cpp to read from file instead of hardcoded string
    // For now, we'll use a workaround by creating a modified version
    const { stdout: mainOutput, stderr: mainError } = await execAsync(
      `cd "${COMPILER_PATH}" && echo '${code.replace(/'/g, "'\\''")}' | ./Main`,
      { timeout: 10000 }
    )

    if (mainError) {
      console.error('Parser stderr:', mainError)
    }

    // ast.json is already in COMPILER_PATH, no need to copy

    // Step 2: Run semantic analyzer
    console.log('Step 2: Running semantic analyzer...')
    const semanticExe = path.join(COMPILER_PATH, 'semantic', 'semantic_main')
    
    const { stdout: semanticOutput, stderr: semanticError } = await execAsync(
      `cd "${path.join(COMPILER_PATH, 'semantic')}" && ./semantic_main "${astFile}" "${TEMP_DIR}"`,
      { timeout: 10000 }
    )

    if (semanticError) {
      console.error('Semantic stderr:', semanticError)
    }

    // Step 3: Run code generator
    console.log('Step 3: Running code generator...')
    const codegenExe = path.join(COMPILER_PATH, 'codegen', 'codegen')
    
    const { stdout: codegenOutput, stderr: codegenError } = await execAsync(
      `cd "${path.join(COMPILER_PATH, 'codegen')}" && ./codegen "${annotatedAstFile}" "${symbolTableFile}" "${irFile}"`,
      { timeout: 10000 }
    )

    if (codegenError) {
      console.error('Codegen stderr:', codegenError)
    }

    // Read all output files
    const [annotatedAst, symbolTable, tac] = await Promise.all([
      fs.readFile(annotatedAstFile, 'utf-8').then(JSON.parse).catch(() => null),
      fs.readFile(symbolTableFile, 'utf-8').then(JSON.parse).catch(() => null),
      fs.readFile(irFile, 'utf-8').catch(() => null)
    ])

    // Clean up temporary files
    await Promise.all([
      fs.unlink(tempFile).catch(() => {}),
      fs.unlink(path.join(COMPILER_PATH, 'ast.json')).catch(() => {}),
      fs.unlink(annotatedAstFile).catch(() => {}),
      fs.unlink(symbolTableFile).catch(() => {}),
      fs.unlink(irFile).catch(() => {})
    ])

    // Return the results
    res.json({
      ast: annotatedAst,
      symbolTable: symbolTable,
      tac: tac
    })

  } catch (error) {
    console.error('Compilation error:', error)
    
    // Clean up on error
    await Promise.all([
      fs.unlink(tempFile).catch(() => {}),
      fs.unlink(path.join(COMPILER_PATH, 'ast.json')).catch(() => {}),
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
