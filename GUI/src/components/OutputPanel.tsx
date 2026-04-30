import { OutputTab, CompilerOutput, ExecutionStep } from '../App'
import TACView from './TACView'
import ASTView from './ASTView'
import SymbolTableView from './SymbolTableView'
import LearnView from './LearnView'
import ExprTreeView from './ExprTreeView'
<<<<<<< HEAD
=======
import ParseTraceView from './ParseTraceView'
>>>>>>> karim-radwan
import './OutputPanel.css'

interface OutputPanelProps {
  output: CompilerOutput | null
  activeTab: OutputTab
  onTabChange: (tab: OutputTab) => void
  onTacLineClick: (line: number) => void
  activeStep: ExecutionStep | null
  onStepForward: () => void
  onStepBackward: () => void
  executionSteps: ExecutionStep[]
}

function OutputPanel({
  output,
  activeTab,
  onTabChange,
  onTacLineClick,
  activeStep,
  onStepForward,
  onStepBackward,
  executionSteps
}: OutputPanelProps) {
  const getTacLineCount = () => {
    if (!output?.tac) return 0
    return output.tac.split('\n').filter(l => l.trim()).length
  }

  const getSymbolCount = () => {
    if (!output?.symbolTable) return 0
    return Object.keys(output.symbolTable).length
  }

  // Check if there are errors
  const hasErrors = output?.errors && output.errors.length > 0

  return (
    <div className="output-panel">
      {/* Only show tabs if compilation was successful (no errors) */}
      {!hasErrors && (
        <div className="tab-bar">
          <button
            className={`tab ${activeTab === 'tac' ? 'active' : ''}`}
            onClick={() => onTabChange('tac')}
          >
            <span className="tab-icon">⚙️</span>
            <span className="tab-label">TAC</span>
            {output?.tac && (
              <span className="tab-badge">{getTacLineCount()}</span>
            )}
          </button>

          <button
            className={`tab ${activeTab === 'ast' ? 'active' : ''}`}
            onClick={() => onTabChange('ast')}
          >
            <span className="tab-icon">🌳</span>
            <span className="tab-label">AST</span>
          </button>

          <button
            className={`tab ${activeTab === 'symbols' ? 'active' : ''}`}
            onClick={() => onTabChange('symbols')}
          >
            <span className="tab-icon">📋</span>
            <span className="tab-label">Symbols</span>
            {output?.symbolTable && (
              <span className="tab-badge">{getSymbolCount()}</span>
            )}
          </button>

          <button
            className={`tab ${activeTab === 'learn' ? 'active' : ''} tab-highlight`}
            onClick={() => onTabChange('learn')}
          >
            <span className="tab-icon">🧩</span>
            <span className="tab-label">Variable Visualizer</span>
          </button>

          <button
            className={`tab ${activeTab === 'exprtree' ? 'active' : ''}`}
            onClick={() => onTabChange('exprtree')}
          >
            <span className="tab-icon">🌿</span>
            <span className="tab-label">Expr Tree</span>
          </button>
<<<<<<< HEAD
=======

          <button
            className={`tab ${activeTab === 'parsetrace' ? 'active' : ''}`}
            onClick={() => onTabChange('parsetrace')}
          >
            <span className="tab-icon">📜</span>
            <span className="tab-label">Parse Trace</span>
            {output?.parseTrace && (
              <span className="tab-badge">{output.parseTrace.length}</span>
            )}
          </button>
>>>>>>> karim-radwan
        </div>
      )}

      <div className="tab-content">
        {!output ? (
          <div className="empty-state">
            <div className="empty-icon">📝</div>
            <h3>Ready to Compile</h3>
            <p>Click the "Compile" button to analyze your code</p>
            <div className="example-code">
              <h4>Example Code:</h4>
              <pre>{`int main() {
    int x[4] = {1, 2, 3, 4};
    int y = x[0];
    return 0;
}`}</pre>
            </div>
          </div>
        ) : hasErrors ? (
          <div className="error-state">
            <div className="error-header">
              <div className="error-icon">❌</div>
              <h3>Compilation Failed</h3>
              <p>Found {output.errors?.length || 0} error{(output.errors?.length || 0) > 1 ? 's' : ''} in your code</p>
            </div>
            <div className="error-list">
              {(output.errors || []).map((error, index) => (
                <div key={index} className="error-item-detailed">
                  <div className="error-item-header">
                    <span className="error-number">Error {index + 1}</span>
                    <span className="error-phase" style={{
                      color: error.phase === 'syntax' ? 'var(--accent-error)' : 
                             error.phase === 'semantic' ? 'var(--accent-warning)' : 
                             'var(--text-tertiary)'
                    }}>
                      {error.phase.toUpperCase()}
                    </span>
                    {error.line && (
                      <span className="error-line">Line {error.line}</span>
                    )}
                  </div>
                  <div className="error-message-detailed">
                    {error.message}
                  </div>
                </div>
              ))}
            </div>
            <div className="error-footer">
              <p>💡 Fix the errors above and compile again</p>
            </div>
          </div>
        ) : (
          <>
            {activeTab === 'tac' && (
              <TACView
                tac={output.tac}
                optimizedTac={output.optimizedTac}
                onLineClick={onTacLineClick}
                activeStepIndex={activeStep?.tacIndex ?? null}
                onStepForward={onStepForward}
                onStepBackward={onStepBackward}
                totalSteps={executionSteps.length}
              />
            )}
            {activeTab === 'ast' && (
              <ASTView ast={output.ast} />
            )}
            {activeTab === 'symbols' && (
              <SymbolTableView symbolTable={output.symbolTable} />
            )}
            {activeTab === 'learn' && (
              <LearnView output={output} activeStep={activeStep} />
            )}
            {activeTab === 'exprtree' && (
              <ExprTreeView ast={output.ast} />
            )}
<<<<<<< HEAD
=======
            {activeTab === 'parsetrace' && (
              <ParseTraceView steps={output.parseTrace || []} />
            )}
>>>>>>> karim-radwan
          </>
        )}
      </div>
    </div>
  )
}

export default OutputPanel
