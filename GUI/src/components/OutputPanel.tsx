import { useState } from 'react'
import { CompilerOutput } from '../App'
import ASTVisualizer from './ASTVisualizer'
import SymbolTableView from './SymbolTableView'
import TACView from './TACView'
import './OutputPanel.css'

interface OutputPanelProps {
  output: CompilerOutput | null
  isLoading: boolean
}

type Tab = 'ast' | 'symbols' | 'tac'

function OutputPanel({ output, isLoading }: OutputPanelProps) {
  const [activeTab, setActiveTab] = useState<Tab>('ast')

  if (isLoading) {
    return (
      <div className="output-panel">
        <div className="loading-state">
          <div className="spinner"></div>
          <p>Compiling your code...</p>
        </div>
      </div>
    )
  }

  if (!output) {
    return (
      <div className="output-panel">
        <div className="empty-state">
          <div className="empty-icon">📝</div>
          <h3>Ready to Compile</h3>
          <p>Write your code and click "Compile & Run" to see the results</p>
          <div className="example-code">
            <h4>Example:</h4>
            <code>{`int x[2][2] = {{1,2},{3,4}};`}</code>
          </div>
        </div>
      </div>
    )
  }

  if (output.error) {
    return (
      <div className="output-panel">
        <div className="error-state">
          <div className="error-icon">❌</div>
          <h3>Compilation Error</h3>
          <pre className="error-message">{output.error}</pre>
        </div>
      </div>
    )
  }

  return (
    <div className="output-panel">
      <div className="tabs">
        <button
          className={`tab ${activeTab === 'ast' ? 'active' : ''}`}
          onClick={() => setActiveTab('ast')}
        >
          🌳 Abstract Syntax Tree
        </button>
        <button
          className={`tab ${activeTab === 'symbols' ? 'active' : ''}`}
          onClick={() => setActiveTab('symbols')}
        >
          📊 Symbol Table
        </button>
        <button
          className={`tab ${activeTab === 'tac' ? 'active' : ''}`}
          onClick={() => setActiveTab('tac')}
        >
          ⚙️ Three-Address Code
        </button>
      </div>

      <div className="tab-content">
        {activeTab === 'ast' && <ASTVisualizer ast={output.ast} />}
        {activeTab === 'symbols' && <SymbolTableView symbolTable={output.symbolTable} />}
        {activeTab === 'tac' && <TACView tac={output.tac} />}
      </div>
    </div>
  )
}

export default OutputPanel
