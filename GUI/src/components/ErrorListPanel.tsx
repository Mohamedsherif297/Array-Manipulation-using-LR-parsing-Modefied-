import { CompilerError } from '../App'
import './ErrorListPanel.css'

interface ErrorListPanelProps {
  errors: CompilerError[]
  phases?: {
    lexical: boolean
    syntax: boolean
    semantic: boolean
    codegen: boolean
  }
  onErrorClick: (line: number) => void
}

function ErrorListPanel({ errors, phases, onErrorClick }: ErrorListPanelProps) {
  const getPhaseIcon = (passed: boolean) => passed ? '✓' : 'X'
  
  const getPhaseLabel = (phase: string) => {
    const labels: Record<string, string> = {
      lexical: 'Lexical Analysis',
      syntax: 'Syntax Analysis',
      semantic: 'Semantic Analysis',
      codegen: 'Code Generation'
    }
    return labels[phase] || phase
  }

  const groupedErrors = errors.reduce((acc, error) => {
    const phase = error.phase || 'unknown'
    if (!acc[phase]) {
      acc[phase] = []
    }
    acc[phase].push(error)
    return acc
  }, {} as Record<string, CompilerError[]>)

  return (
    <div className="error-list-panel">
      <div className="error-header">
        <div className="error-title">
          <span className="error-icon">X</span>
          <h2>Compilation Failed</h2>
        </div>
        <div className="error-count">
          {errors.length} error{errors.length !== 1 ? 's' : ''} found
        </div>
      </div>

      {phases && (
        <div className="phase-status">
          <h3>Compilation Phases</h3>
          <div className="phase-grid">
            {Object.entries(phases).map(([phase, passed]) => (
              <div key={phase} className={`phase-card ${passed ? 'passed' : 'failed'}`}>
                <span className="phase-icon">{getPhaseIcon(passed)}</span>
                <span className="phase-name">{getPhaseLabel(phase)}</span>
              </div>
            ))}
          </div>
        </div>
      )}

      <div className="errors-container">
        {Object.entries(groupedErrors).map(([phase, phaseErrors]) => (
          <div key={phase} className="error-group">
            <div className="error-group-header">
              <span className={`phase-badge phase-${phase}`}>
                {phase.toUpperCase()}
              </span>
              <span className="error-group-count">
                {phaseErrors.length} error{phaseErrors.length !== 1 ? 's' : ''}
              </span>
            </div>
            <div className="error-list">
              {phaseErrors.map((error, index) => (
                <div 
                  key={index} 
                  className="error-item"
                  onClick={() => error.line && onErrorClick(error.line)}
                  style={{ cursor: error.line ? 'pointer' : 'default' }}
                >
                  {error.line && (
                    <div className="error-line-number">
                      Line {error.line}
                    </div>
                  )}
                  <div className="error-message">
                    {error.message}
                  </div>
                  {error.line && (
                    <div className="error-action">
                      Click to jump to line →
                    </div>
                  )}
                </div>
              ))}
            </div>
          </div>
        ))}
      </div>

      <div className="error-help">
        <h3>Quick Tips</h3>
        <ul>
          <li><strong>Syntax Errors:</strong> Check for missing semicolons, brackets, or invalid syntax</li>
          <li><strong>Semantic Errors:</strong> Check for type mismatches, undeclared variables, or array bounds</li>
          <li>Click on any error with a line number to jump to that line in the code</li>
          <li>Fix errors from top to bottom - earlier errors may cause later ones</li>
        </ul>
      </div>
    </div>
  )
}

export default ErrorListPanel
