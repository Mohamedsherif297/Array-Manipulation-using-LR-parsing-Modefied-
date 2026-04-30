import { useEffect, useState, useCallback } from 'react'
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

export interface ExecutionStep {
  tacIndex: number
  sourceLine: number
  affectedVars: string[]
}

export type CompilationStatus = 'idle' | 'compiling' | 'success' | 'error'
export type OutputTab = 'tac' | 'ast' | 'symbols' | 'learn' | 'exprtree' | 'parsetrace'

// ── Editor tab (file tab) ────────────────────────────────────────────────────
export interface EditorTab {
  id: string
  label: string
  code: string
  output: CompilerOutput | null
  status: CompilationStatus
  undoStack: string[]
  redoStack: string[]
}

let tabCounter = 1
function makeTab(label?: string, code?: string): EditorTab {
  return {
    id: `tab-${Date.now()}-${tabCounter++}`,
    label: label ?? `main${tabCounter - 1}.c`,
    code: code ?? `int main() {\n    return 0;\n}`,
    output: null,
    status: 'idle',
    undoStack: [],
    redoStack: [],
  }
}

const DEFAULT_CODE = `int main() {
    int x[4] = {1, 2, 3, 4};
    int y[2][3] = {{1, 2, 3}, {4, 5, 6}};
    int a = x[0];
    int b = y[1][2];
    return 0;
}`

function buildExecutionSteps(tac: string): ExecutionStep[] {
  const steps: ExecutionStep[] = []
  const lines = tac.split('\n').filter(l => l.trim())
  lines.forEach((line, idx) => {
    const lineMatch = line.match(/;\s*line:(\d+)\s*$/)
    const sourceLine = lineMatch ? parseInt(lineMatch[1]) : 0
    const clean = line.replace(/;\s*line:\d+\s*$/, '').trim()
    const vars = new Set<string>()
    if (!clean.startsWith('//')) {
      const lhsMatch = clean.match(/^([a-zA-Z_]\w*)(?:\[|[\s=])/)
      if (lhsMatch) vars.add(lhsMatch[1])
      const rhsMatch = clean.match(/=\s*(.+)$/)
      if (rhsMatch) {
        const ids = rhsMatch[1].match(/[a-zA-Z_]\w*/g) || []
        ids.forEach(id => { if (!id.startsWith('t') || isNaN(Number(id.slice(1)))) vars.add(id) })
      }
      const printMatch = clean.match(/^(?:PRINT|READ)\s+([a-zA-Z_]\w*)/)
      if (printMatch) vars.add(printMatch[1])
    }
    steps.push({ tacIndex: idx, sourceLine, affectedVars: [...vars] })
  })
  return steps
}

function App() {
  // ── Tab state ──────────────────────────────────────────────────────────────
  const [tabs, setTabs] = useState<EditorTab[]>([
    { ...makeTab('main.c', DEFAULT_CODE), id: 'tab-default' }
  ])
  const [activeTabId, setActiveTabId] = useState<string>('tab-default')
  const [renamingTabId, setRenamingTabId] = useState<string | null>(null)
  const [renameValue, setRenameValue] = useState('')

  const activeTab = tabs.find(t => t.id === activeTabId) ?? tabs[0]

  // ── Shared UI state ────────────────────────────────────────────────────────
  const [currentPhase, setCurrentPhase] = useState<string>('None')
  const [compilationTime, setCompilationTime] = useState<number>(0)
  const [activeOutputTab, setActiveOutputTab] = useState<OutputTab>('tac')
  const [highlightedLine, setHighlightedLine] = useState<number | null>(null)
  const [problemsPanelHeight, setProblemsPanelHeight] = useState(200)
  const [isDarkMode, setIsDarkMode] = useState(true)
  const [consoleInput, setConsoleInput] = useState('')
  const [consoleHistory, setConsoleHistory] = useState('')
  const [executionSteps, setExecutionSteps] = useState<ExecutionStep[]>([])
  const [activeStepIndex, setActiveStepIndex] = useState<number | null>(null)

  const activeStep = activeStepIndex !== null ? executionSteps[activeStepIndex] ?? null : null

  useEffect(() => {
    document.documentElement.setAttribute('data-theme', isDarkMode ? 'dark' : 'light')
  }, [isDarkMode])

  // ── Tab management ─────────────────────────────────────────────────────────
  const addTab = () => {
    const t = makeTab()
    setTabs(prev => [...prev, t])
    setActiveTabId(t.id)
    setHighlightedLine(null)
    setExecutionSteps([])
    setActiveStepIndex(null)
  }

  const closeTab = (id: string, e: React.MouseEvent) => {
    e.stopPropagation()
    setTabs(prev => {
      if (prev.length === 1) return prev // keep at least one tab
      const idx = prev.findIndex(t => t.id === id)
      const next = prev.filter(t => t.id !== id)
      if (id === activeTabId) {
        const newActive = next[Math.max(0, idx - 1)]
        setActiveTabId(newActive.id)
      }
      return next
    })
  }

  const startRename = (id: string, label: string, e: React.MouseEvent) => {
    e.stopPropagation()
    setRenamingTabId(id)
    setRenameValue(label)
  }

  const commitRename = () => {
    if (!renamingTabId) return
    const trimmed = renameValue.trim()
    if (trimmed) {
      setTabs(prev => prev.map(t => t.id === renamingTabId ? { ...t, label: trimmed } : t))
    }
    setRenamingTabId(null)
  }

  // ── Per-tab code / undo / redo ─────────────────────────────────────────────
  const updateActiveTab = useCallback((patch: Partial<EditorTab>) => {
    setTabs(prev => prev.map(t => t.id === activeTabId ? { ...t, ...patch } : t))
  }, [activeTabId])

  const handleCodeChange = (nextCode: string) => {
    if (nextCode === activeTab.code) return
    updateActiveTab({
      code: nextCode,
      undoStack: [...activeTab.undoStack, activeTab.code],
      redoStack: [],
    })
  }

  const handleUndo = () => {
    if (activeTab.undoStack.length === 0) return
    const previous = activeTab.undoStack[activeTab.undoStack.length - 1]
    updateActiveTab({
      code: previous,
      undoStack: activeTab.undoStack.slice(0, -1),
      redoStack: [...activeTab.redoStack, activeTab.code],
    })
  }

  const handleRedo = () => {
    if (activeTab.redoStack.length === 0) return
    const next = activeTab.redoStack[activeTab.redoStack.length - 1]
    updateActiveTab({
      code: next,
      undoStack: [...activeTab.undoStack, activeTab.code],
      redoStack: activeTab.redoStack.slice(0, -1),
    })
  }

  // ── Compilation ────────────────────────────────────────────────────────────
  const handleCompile = async () => {
    updateActiveTab({ status: 'compiling', output: null })
    setHighlightedLine(null)
    const startTime = Date.now()

    try {
      setCurrentPhase('Lexical Analysis')
      await new Promise(resolve => setTimeout(resolve, 200))
      setCurrentPhase('Syntax Analysis')
      await new Promise(resolve => setTimeout(resolve, 200))

      const response = await fetch('http://localhost:3003/api/compile', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ code: activeTab.code, stdin: consoleInput }),
      })
      const result = await response.json()

      setCurrentPhase('Semantic Analysis')
      await new Promise(resolve => setTimeout(resolve, 200))
      setCurrentPhase('Code Generation')
      await new Promise(resolve => setTimeout(resolve, 200))

      const endTime = Date.now()
      setCompilationTime(endTime - startTime)

      const runLog = ['---- Run ----', result.consoleOutput?.trimEnd() || '(no output)'].join('\n')
      setConsoleHistory(prev => prev ? `${prev.trimEnd()}\n${runLog}\n` : `${runLog}\n`)

      if (result.success) {
        setCurrentPhase('Complete')
        updateActiveTab({ status: 'success', output: result })
        if (result.tac) {
          setExecutionSteps(buildExecutionSteps(result.tac))
          setActiveStepIndex(null)
        }
        if (result.warnings?.some((w: string) => w.includes('cin'))) {
          setProblemsPanelHeight(220)
        }
      } else {
        setCurrentPhase('Failed')
        updateActiveTab({ status: 'error', output: result })
        setExecutionSteps([])
        setActiveStepIndex(null)
      }
    } catch (error) {
      setCompilationTime(Date.now() - startTime)
      setCurrentPhase('Failed')
      updateActiveTab({
        status: 'error',
        output: { errors: [{ message: `Network error: ${error}`, phase: 'unknown' }], success: false },
      })
    }
  }

  const handleReset = () => {
    updateActiveTab({ output: null, status: 'idle' })
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

  return (
    <div className="app">
      <Header
        status={activeTab.status}
        currentPhase={currentPhase}
        compilationTime={compilationTime}
        onCompile={handleCompile}
        onReset={handleReset}
        onThemeToggle={() => setIsDarkMode(p => !p)}
        isDarkMode={isDarkMode}
      />

      <div className="main-workspace">
        <EditorPanel
          code={activeTab.code}
          onChange={handleCodeChange}
          highlightedLine={highlightedLine}
          errors={activeTab.output?.errors}
          onLineClick={setHighlightedLine}
          canUndo={activeTab.undoStack.length > 0}
          canRedo={activeTab.redoStack.length > 0}
          onUndo={handleUndo}
          onRedo={handleRedo}
          // Tab bar props
          tabs={tabs}
          activeTabId={activeTabId}
          renamingTabId={renamingTabId}
          renameValue={renameValue}
          onTabSelect={(id) => { setActiveTabId(id); setHighlightedLine(null); setExecutionSteps([]); setActiveStepIndex(null) }}
          onTabAdd={addTab}
          onTabClose={closeTab}
          onTabRenameStart={startRename}
          onTabRenameChange={setRenameValue}
          onTabRenameCommit={commitRename}
        />

        <OutputPanel
          output={activeTab.output}
          activeTab={activeOutputTab}
          onTabChange={setActiveOutputTab}
          onTacLineClick={handleTacLineClick}
          activeStep={activeStep}
          onStepForward={handleStepForward}
          onStepBackward={handleStepBackward}
          executionSteps={executionSteps}
        />
      </div>

      <ProblemsPanel
        errors={activeTab.output?.errors || []}
        warnings={activeTab.output?.warnings || []}
        height={problemsPanelHeight}
        onHeightChange={setProblemsPanelHeight}
        onErrorClick={setHighlightedLine}
        consoleOutput={consoleHistory || activeTab.output?.consoleOutput || ''}
        consoleInput={consoleInput}
        onConsoleInputChange={setConsoleInput}
        onClearConsole={() => setConsoleHistory('')}
      />
    </div>
  )
}

export default App
