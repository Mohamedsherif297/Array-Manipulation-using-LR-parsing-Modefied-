import { useState } from 'react'
import CodeEditor from './components/CodeEditor'
import OutputPanel from './components/OutputPanel'
import './App.css'

export interface CompilerOutput {
  ast?: any
  symbolTable?: any
  tac?: string
  error?: string
}

function App() {
  const [code, setCode] = useState(`int x[2][2] = {{1,2},{3,4}};`)
  const [output, setOutput] = useState<CompilerOutput | null>(null)
  const [isCompiling, setIsCompiling] = useState(false)

  const handleCompile = async () => {
    setIsCompiling(true)
    setOutput(null)

    try {
      const response = await fetch('/api/compile', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({ code }),
      })

      const result = await response.json()
      
      if (response.ok) {
        setOutput(result)
      } else {
        setOutput({ error: result.error || 'Compilation failed' })
      }
    } catch (error) {
      setOutput({ error: `Network error: ${error}` })
    } finally {
      setIsCompiling(false)
    }
  }

  return (
    <div className="app">
      <header className="app-header">
        <h1>🔧 Array Manipulation Compiler</h1>
        <p>LR Parser with Semantic Analysis & Code Generation</p>
      </header>
      
      <div className="app-content">
        <div className="editor-section">
          <div className="section-header">
            <h2>Source Code</h2>
            <button 
              className="compile-btn" 
              onClick={handleCompile}
              disabled={isCompiling}
            >
              {isCompiling ? '⏳ Compiling...' : '▶️ Compile & Run'}
            </button>
          </div>
          <CodeEditor value={code} onChange={setCode} />
        </div>

        <div className="output-section">
          <OutputPanel output={output} isLoading={isCompiling} />
        </div>
      </div>
    </div>
  )
}

export default App
