import { useEffect, useState } from 'react'
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
  consoleOutput?: string
  runtimeError?: string | null
  errors?: CompilerError[]
  warnings?: string[]
  success?: boolean
  parseTrace?: Array<{ stack: string; input: string; action: string }>
  phases?: {
    lexical: boolean
    syntax: boolean
    semantic: boolean
    codegen: boolean
  }
}

// One TAC instruction mapped to its source context
export interface ExecutionStep {
  tacIndex: number       // index in the TAC lines array
  sourceLine: number     // source code line (0 = unknown)
  affectedVars: string[] // variable names written/read in this instruction
}

export type CompilationStatus = 'idle' | 'compiling' | 'success' | 'error'
export type OutputTab = 'tac' | 'ast' | 'symbols' | 'learn' | 'exprtree' | 'parsetrace'

// Parse TAC text into ExecutionStep timeline
function buildExecutionSteps(tac: string): ExecutionStep[] {
  const steps: ExecutionStep[] = []
  const lines = tac.split('\n').filter(l => l.trim())

  lines.forEach((line, idx) => {
    // Extract source line from trailing annotation:  ; line:N
    const lineMatch = line.match(/;\s*line:(\d+)\s*$/)
    const sourceLine = lineMatch ? parseInt(lineMatch[1]) : 0

    // Extract affected variable names from the instruction
    const clean = line.replace(/;\s*line:\d+\s*$/, '').trim()
    const vars = new Set<string>()

    // Skip pure comments
    if (clean.startsWith('//')) {
      steps.push({ tacIndex: idx, sourceLine, affectedVars: [] })
      return
    }

    // LHS of assignment:  "x = ..." or "x[...] = ..."  or "arr[i] = val"
    const lhsMatch = clean.match(/^([a-zA-Z_]\w*)(?:\[|[\s=])/)
    if (lhsMatch) vars.add(lhsMatch[1])

    // RHS identifiers (not temporaries, not numbers)
    const rhsMatch = clean.match(/=\s*(.+)$/)
    if (rhsMatch) {
      const rhs = rhsMatch[1]
      const ids = rhs.match(/[a-zA-Z_]\w*/g) || []
      ids.forEach(id => {
        if (!id.startsWith('t') || isNaN(Number(id.slice(1)))) {
          vars.add(id)
        }
      })
    }

    // PRINT / READ targets
    const printMatch = clean.match(/^(?:PRINT|READ)\s+([a-zA-Z_]\w*)/)
    if (printMatch) vars.add(printMatch[1])

    steps.push({ tacIndex: idx, sourceLine, affectedVars: [...vars] })
  })

  return steps
}

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
  const [consoleInput, setConsoleInput] = useState('')
  const [consoleHistory, setConsoleHistory] = useState('')
  const [undoStack, setUndoStack] = useState<string[]>([])
  const [redoStack, setRedoStack] = useState<string[]>([])

  // Execution stepper state
  const [executionSteps, setExecutionSteps] = useState<ExecutionStep[]>([])
  const [activeStepIndex, setActiveStepIndex] = useState<number | null>(null)

  const activeStep = activeStepIndex !== null ? executionSteps[activeStepIndex] ?? null : null

  useEffect(() => {
    document.documentElement.setAttribute('data-theme', isDarkMode ? 'dark' : 'light')
  }, [isDarkMode])

  const handleCodeChange = (nextCode: string) => {
    if (nextCode === code) return
    setUndoStack(prev => [...prev, code])
    setRedoStack([])
    setCode(nextCode)
  }

  const handleUndo = () => {
    if (undoStack.length === 0) return
    const previous = undoStack[undoStack.length - 1]
    setUndoStack(prev => prev.slice(0, -1))
    setRedoStack(prev => [...prev, code])
    setCode(previous)
  }

  const handleRedo = () => {
    if (redoStack.length === 0) return
    const next = redoStack[redoStack.length - 1]
    setRedoStack(prev => prev.slice(0, -1))
    setUndoStack(prev => [...prev, code])
    setCode(next)
  }

  const handleThemeToggle = () => {
    setIsDarkMode(prev => !prev)
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
        body: JSON.stringify({ code, stdin: consoleInput }),
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

      const runLog = [
        '---- Run ----',
        result.consoleOutput?.trimEnd() || '(no output)',
      ].join('\n')
      setConsoleHistory(prev => prev ? `${prev.trimEnd()}\n${runLog}\n` : `${runLog}\n`)
      
      if (result.success) {
        setStatus('success')
        setCurrentPhase('Complete')
        setOutput(result)
        // Build execution step timeline from TAC
        if (result.tac) {
          setExecutionSteps(buildExecutionSteps(result.tac))
          setActiveStepIndex(null)
        }
      } else {
        setStatus('error')
        setCurrentPhase('Failed')
        setOutput(result)
        setExecutionSteps([])
        setActiveStepIndex(null)
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
    setExecutionSteps([])
    setActiveStepIndex(null)
  }

  const handleTacLineClick = (tacIndex: number) => {
    setActiveStepIndex(tacIndex)
    const step = executionSteps[tacIndex]
    if (step?.sourceLine) setHighlightedLine(step.sourceLine)
  }

  const handleStepForward = () => {
    setActiveStepIndex(prev => {
      const next = prev === null ? 0 : Math.min(prev + 1, executionSteps.length - 1)
      const step = executionSteps[next]
      if (step?.sourceLine) setHighlightedLine(step.sourceLine)
      return next
    })
  }

  const handleStepBackward = () => {
    setActiveStepIndex(prev => {
      if (prev === null || prev === 0) return null
      const next = prev - 1
      const step = executionSteps[next]
      if (step?.sourceLine) setHighlightedLine(step.sourceLine)
      return next
    })
  }

  const handleClearConsole = () => {
    setConsoleHistory('')
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
          onChange={handleCodeChange}
          highlightedLine={highlightedLine}
          errors={output?.errors}
          onLineClick={setHighlightedLine}
          canUndo={undoStack.length > 0}
          canRedo={redoStack.length > 0}
          onUndo={handleUndo}
          onRedo={handleRedo}
        />
        
        <OutputPanel 
          output={output}
          activeTab={activeTab}
          onTabChange={setActiveTab}
          onTacLineClick={handleTacLineClick}
          activeStep={activeStep}
          onStepForward={handleStepForward}
          onStepBackward={handleStepBackward}
          executionSteps={executionSteps}
        />
      </div>

      <ProblemsPanel 
        errors={output?.errors || []}
        warnings={output?.warnings || []}
        height={problemsPanelHeight}
        onHeightChange={setProblemsPanelHeight}
        onErrorClick={handleErrorClick}
        consoleOutput={consoleHistory || output?.consoleOutput || ''}
        consoleInput={consoleInput}
        onConsoleInputChange={setConsoleInput}
        onClearConsole={handleClearConsole}
      />
    </div>
  )
}

export default App
