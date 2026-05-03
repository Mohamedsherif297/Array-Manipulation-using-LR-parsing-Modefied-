import { CompilerOutput } from '../App'
import './CompileFeedbackPanel.css'

interface CompileFeedbackPanelProps {
  output: CompilerOutput
  onErrorClick: (line: number) => void
}

function CompileFeedbackPanel({ output, onErrorClick }: CompileFeedbackPanelProps) {
  const { errors = [], warnings = [], success, phases } = output

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

  return (
    <div className="compile-feedback-panel">
      <div className="feedback-header">
        <h3>
          {success ? 'Compilation Successful' : 'Compilation Failed'}
        </h3>
        <div className="phase-indicators">
          {phases && Object.entries(phases).map(([phase, passed]) => (
            <div key={phase} className={`phase-indicator ${passed ? 'passed' : 'failed'}`}>
              <span className="phase-icon">{getPhaseIcon(passed)}</span>
              <span className="phase-name">{getPhaseLabel(phase)}</span>
            </div>
          ))}
        </div>
      </div>

      {errors.length > 0 && (
        <div className="feedback-section errors">
          <h4>🔴 Errors ({errors.length})</h4>
          <div className="feedback-list">
            {errors.map((error, index) => (
              <div 
                key={index} 
                className="feedback-item error-item"
                onClick={() => error.line && onErrorClick(error.line)}
                style={{ cursor: error.line ? 'pointer' : 'default' }}
              >
                <div className="feedback-badge error-badge">
                  {error.phase.toUpperCase()}
                </div>
                <div className="feedback-content">
                  {error.line && (
                    <span className="feedback-line">Line {error.line}: </span>
                  )}
                  <span className="feedback-message">{error.message}</span>
                </div>
              </div>
            ))}
          </div>
        </div>
      )}

      {warnings.length > 0 && (
        <div className="feedback-section warnings">
          <h4>Warnings ({warnings.length})</h4>
          <div className="feedback-list">
            {warnings.map((warning, index) => (
              <div key={index} className="feedback-item warning-item">
                <div className="feedback-badge warning-badge">WARNING</div>
                <div className="feedback-content">
                  <span className="feedback-message">{warning}</span>
                </div>
              </div>
            ))}
          </div>
        </div>
      )}

      {success && errors.length === 0 && warnings.length === 0 && (
        <div className="feedback-section success">
          <div className="success-message">
            <span className="success-icon">🎉</span>
            <p>Your code compiled successfully with no errors or warnings!</p>
          </div>
        </div>
      )}
    </div>
  )
}

export default CompileFeedbackPanel
