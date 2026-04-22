import { useState } from 'react'
import Header from './components/Header'
import EditorPanel from './components/EditorPanel'
import OutputPanel from './components/OutputPanel'
import ProblemsPanel from './components/ProblemsPanel'
import './App.css'

export interface CompilerError {
  message: string
  line?: number
  phase: 'lexical' | 'syntax' | 'semantic' | 'codegen' | 'unknown'
}

export interface CompilerOutput {
  ast?: any
  symbolTable?: any
  tac?: string
  optimizedTac?: string
  errors?: CompilerError[]
  warnings?: string[]
  success?: boolean
  phases?: {
    lexical: boolean
    syntax: boolean
    semantic: boolean
    codegen: boolean
  }
}

export type CompilationStatus = 'idle' | 'compiling' | 'success' | 'error'
export type OutputTab = 'tac' | 'ast' | 'symbols' | 'learn'

function App() {
  const [code, setCode] = useState(`int main() {
    int x[4] = {1, 2, 3, 4};
    int y[2][3] = {{1, 2, 3}, {4, 5, 6}};
    int a = x[0];
    int b = y[1][2];
    return 0;
}`)
  const [output, setOutput] = useState<CompilerOutput | null>(null)
  const [status, setStatus] = useState<CompilationStatus>('idle')
  const [currentPhase, setCurrentPhase] = useState<string>('None')
  const [compilationTime, setCompilationTime] = useState<number>(0)
  const [activeTab, setActiveTab] = useState<OutputTab>('tac')
  const [highlightedLine, setHighlightedLine] = useState<number | null>(null)
  const [problemsPanelHeight, setProblemsPanelHeight] = useState(200)
  const [isDarkMode, setIsDarkMode] = useState(true)

  const handleThemeToggle = () => {
    setIsDarkMode(!isDarkMode)
    document.documentElement.setAttribute('data-theme', isDarkMode ? 'light' : 'dark')
  }

  const handleCompile = async () => {
    setStatus('compiling')
    setOutput(null)
    setHighlightedLine(null)
    
    const startTime = Date.now()

    try {
      setCurrentPhase('Lexical Analysis')
      await new Promise(resolve => setTimeout(resolve, 200))
      
      setCurrentPhase('Syntax Analysis')
      await new Promise(resolve => setTimeout(resolve, 200))

      const response = await fetch('http://localhost:3003/api/compile', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({ code }),
      })

      const result = await response.json()
      
      console.log('Received compilation result:', result)
      console.log('Errors received:', result.errors)
      
      setCurrentPhase('Semantic Analysis')
      await new Promise(resolve => setTimeout(resolve, 200))
      
      setCurrentPhase('Code Generation')
      await new Promise(resolve => setTimeout(resolve, 200))

      const endTime = Date.now()
      setCompilationTime(endTime - startTime)
      
      if (result.success) {
        setStatus('success')
        setCurrentPhase('Complete')
        setOutput(result)
      } else {
        setStatus('error')
        setCurrentPhase('Failed')
        setOutput(result)
      }
    } catch (error) {
      const endTime = Date.now()
      setCompilationTime(endTime - startTime)
      setStatus('error')
      setCurrentPhase('Failed')
      setOutput({ 
        errors: [{ message: `Network error: ${error}`, phase: 'unknown' }],
        success: false
      })
    }
  }

  const handleReset = () => {
    setOutput(null)
    setStatus('idle')
    setCurrentPhase('None')
    setCompilationTime(0)
    setHighlightedLine(null)
  }

  const handleErrorClick = (line: number) => {
    setHighlightedLine(line)
  }

  return (
    <div className="app">
      <Header 
        status={status}
        currentPhase={currentPhase}
        compilationTime={compilationTime}
        onCompile={handleCompile}
        onReset={handleReset}
        onThemeToggle={handleThemeToggle}
        isDarkMode={isDarkMode}
      />
      
      <div className="main-workspace">
        <EditorPanel 
          code={code}
          onChange={setCode}
          highlightedLine={highlightedLine}
          errors={output?.errors}
          onLineClick={setHighlightedLine}
        />
        
        <OutputPanel 
          output={output}
          activeTab={activeTab}
          onTabChange={setActiveTab}
          onTacLineClick={() => {}}
        />
      </div>

      <ProblemsPanel 
        errors={output?.errors || []}
        warnings={output?.warnings || []}
        height={problemsPanelHeight}
        onHeightChange={setProblemsPanelHeight}
        onErrorClick={handleErrorClick}
      />
    </div>
  )
}

export default App
