import { useState, useEffect, useRef } from 'react'
import { CompilerError } from '../App'
import './ProblemsPanel.css'

interface ProblemsPanelProps {
  errors: CompilerError[]
  warnings: string[]
  height: number
  onHeightChange: (height: number) => void
  onErrorClick: (line: number) => void
  consoleOutput: string
  consoleInput: string
  onConsoleInputChange: (value: string) => void
  onClearConsole: () => void
}

type ProblemTab = 'problems' | 'warnings' | 'output' | 'console'

function ProblemsPanel({ errors, warnings, height, onHeightChange, onErrorClick, consoleOutput, consoleInput, onConsoleInputChange, onClearConsole }: ProblemsPanelProps) {
  const [activeTab, setActiveTab] = useState<ProblemTab>('problems')
  const [isResizing, setIsResizing] = useState(false)
  const [isCollapsed, setIsCollapsed] = useState(false)
  const consoleScrollRef = useRef<HTMLDivElement>(null)

  const MIN_HEIGHT = 40  // Just show the header when collapsed
  const MAX_HEIGHT = 400
  const DEFAULT_HEIGHT = 200

  useEffect(() => {
    const handleMouseMove = (e: MouseEvent) => {
      if (isResizing) {
        const newHeight = window.innerHeight - e.clientY
        
        // If dragged below minimum, collapse it
        if (newHeight < 100) {
          setIsCollapsed(true)
          onHeightChange(MIN_HEIGHT)
        } else if (newHeight >= 100 && newHeight <= MAX_HEIGHT) {
          setIsCollapsed(false)
          onHeightChange(newHeight)
        } else if (newHeight > MAX_HEIGHT) {
          setIsCollapsed(false)
          onHeightChange(MAX_HEIGHT)
        }
      }
    }

    const handleMouseUp = () => {
      setIsResizing(false)
    }

    if (isResizing) {
      window.addEventListener('mousemove', handleMouseMove)
      window.addEventListener('mouseup', handleMouseUp)
    }

    return () => {
      window.removeEventListener('mousemove', handleMouseMove)
      window.removeEventListener('mouseup', handleMouseUp)
    }
  }, [isResizing, onHeightChange])

  useEffect(() => {
    if (consoleScrollRef.current) {
      consoleScrollRef.current.scrollTop = consoleScrollRef.current.scrollHeight
    }
  }, [consoleOutput, activeTab])

  const handleMouseDown = () => {
    setIsResizing(true)
  }

  const toggleCollapse = () => {
    if (isCollapsed) {
      setIsCollapsed(false)
      onHeightChange(DEFAULT_HEIGHT)
    } else {
      setIsCollapsed(true)
      onHeightChange(MIN_HEIGHT)
    }
  }

  const getPhaseColor = (phase: string) => {
    switch (phase) {
      case 'syntax': return 'var(--accent-error)'
      case 'semantic': return 'var(--accent-warning)'
      case 'lexical': return 'var(--accent-info)'
      case 'codegen': return 'var(--accent-primary)'
      default: return 'var(--text-tertiary)'
    }
  }

  const getPhaseIcon = (phase: string) => {
    switch (phase) {
      case 'syntax': return '⚠️'
      case 'semantic': return '🔍'
      case 'lexical': return '📝'
      case 'codegen': return '⚙️'
      default: return '❌'
    }
  }

  return (
    <div className={`problems-panel ${isCollapsed ? 'collapsed' : ''}`} style={{ height: `${height}px` }}>
      <div 
        className="resize-handle" 
        onMouseDown={handleMouseDown}
        title="Drag to resize, drag down to collapse"
      />
      
      <div className="problems-header">
        <div className="problems-tabs">
          <button
            className={`problems-tab ${activeTab === 'problems' ? 'active' : ''}`}
            onClick={() => !isCollapsed && setActiveTab('problems')}
            disabled={isCollapsed}
          >
            <span className="tab-icon">❌</span>
            <span className="tab-label">Problems</span>
            {errors.length > 0 && (
              <span className="tab-count error">{errors.length}</span>
            )}
          </button>

          <button
            className={`problems-tab ${activeTab === 'warnings' ? 'active' : ''}`}
            onClick={() => !isCollapsed && setActiveTab('warnings')}
            disabled={isCollapsed}
          >
            <span className="tab-icon">⚠</span>
            <span className="tab-label">Warnings</span>
            {warnings.length > 0 && (
              <span className="tab-count warning">{warnings.length}</span>
            )}
          </button>

          <button
            className={`problems-tab ${activeTab === 'output' ? 'active' : ''}`}
            onClick={() => !isCollapsed && setActiveTab('output')}
            disabled={isCollapsed}
          >
            <span className="tab-icon">📋</span>
            <span className="tab-label">Output</span>
          </button>

          <button
            className={`problems-tab ${activeTab === 'console' ? 'active' : ''}`}
            onClick={() => !isCollapsed && setActiveTab('console')}
            disabled={isCollapsed}
          >
            <span className="tab-icon">🔍</span>
            <span className="tab-label">Console</span>
          </button>
        </div>

        <div className="problems-actions">
          <button 
            className="icon-btn collapse-btn" 
            onClick={toggleCollapse}
            title={isCollapsed ? "Expand panel" : "Collapse panel"}
          >
            <span>{isCollapsed ? '▲' : '▼'}</span>
          </button>
          <button className="icon-btn" title="Clear All">
            <span onClick={onClearConsole}>🗑️</span>
          </button>
          <button className="icon-btn" title="Filter">
            <span>🔽</span>
          </button>
        </div>
      </div>

      {!isCollapsed && (
        <div className="problems-content">
          {activeTab === 'problems' && (
            <div className="problems-list">
              {errors.length === 0 ? (
                <div className="empty-problems">
                  <span className="empty-icon">✅</span>
                  <span className="empty-text">No problems detected</span>
                </div>
              ) : (
                errors.map((error, index) => (
                  <div
                    key={index}
                    className="problem-item"
                    onClick={() => error.line && onErrorClick(error.line)}
                  >
                    <div className="problem-icon" style={{ color: getPhaseColor(error.phase) }}>
                      {getPhaseIcon(error.phase)}
                    </div>
                    <div className="problem-details">
                      <div className="problem-message">{error.message}</div>
                      <div className="problem-meta">
                        <span className="problem-phase" style={{ color: getPhaseColor(error.phase) }}>
                          {error.phase.toUpperCase()}
                        </span>
                        {error.line && (
                          <>
                            <span className="meta-separator">•</span>
                            <span className="problem-location">Line {error.line}</span>
                          </>
                        )}
                      </div>
                    </div>
                    {error.line && (
                      <div className="problem-action">
                        <span>→</span>
                      </div>
                    )}
                  </div>
                ))
              )}
            </div>
          )}

          {activeTab === 'warnings' && (
            <div className="problems-list">
              {warnings.length === 0 ? (
                <div className="empty-problems">
                  <span className="empty-icon">✅</span>
                  <span className="empty-text">No warnings</span>
                </div>
              ) : (
                warnings.map((warning, index) => (
                  <div key={index} className="problem-item">
                    <div className="problem-icon" style={{ color: 'var(--accent-warning)' }}>
                      ⚠️
                    </div>
                    <div className="problem-details">
                      <div className="problem-message">{warning}</div>
                    </div>
                  </div>
                ))
              )}
            </div>
          )}

          {activeTab === 'output' && (
            <div className="console-output" ref={consoleScrollRef}>
              {consoleOutput ? (
                consoleOutput.split('\n').map((line, index) => (
                  <div key={index} className="console-line">{line}</div>
                ))
              ) : (
                <div className="console-line">No runtime output yet.</div>
              )}
            </div>
          )}

          {activeTab === 'console' && (
            <div className="console-output" ref={consoleScrollRef}>
              <div className="console-line console-hint">
                💡 If your program uses <strong>cin</strong>, enter space-separated values below, then click <strong>Compile</strong>.
              </div>
              <textarea
                className={`console-input-area ${warnings.some(w => w.includes('cin')) ? 'cin-needed' : ''}`}
                value={consoleInput}
                onChange={(e) => onConsoleInputChange(e.target.value)}
                placeholder="Enter cin input values here (space or newline separated)&#10;Example: 5 9 Karim"
              />
              <div className="console-line console-output-label">── Runtime Output ──</div>
              {consoleOutput ? (
                consoleOutput.split('\n').map((line, index) => (
                  <div key={`console-out-${index}`} className="console-line">{line}</div>
                ))
              ) : (
                <div className="console-line console-dim">(no output yet)</div>
              )}
            </div>
          )}
        </div>
      )}
    </div>
  )
}

export default ProblemsPanel
