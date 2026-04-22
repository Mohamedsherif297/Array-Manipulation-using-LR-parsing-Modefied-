import { useState } from 'react'
import './SymbolTableView.css'

interface SymbolTableViewProps {
  symbolTable: any
}

function SymbolTableView({ symbolTable }: SymbolTableViewProps) {
  const [viewMode, setViewMode] = useState<'table' | 'cards'>('table')
  const [searchTerm, setSearchTerm] = useState('')

  if (!symbolTable || Object.keys(symbolTable).length === 0) {
    return (
      <div className="symbol-table-empty">
        <p>No symbols available. Compile your code first.</p>
      </div>
    )
  }

  const symbols = Object.entries(symbolTable).map(([name, data]: [string, any]) => ({
    name,
    ...data
  }))

  const filteredSymbols = symbols.filter(symbol =>
    symbol.name.toLowerCase().includes(searchTerm.toLowerCase())
  )

  return (
    <div className="symbol-table-view">
      <div className="symbol-table-toolbar">
        <div className="toolbar-section">
          <input
            type="text"
            className="search-box"
            placeholder="Filter symbols..."
            value={searchTerm}
            onChange={(e) => setSearchTerm(e.target.value)}
          />
        </div>
        <div className="toolbar-section">
          <div className="view-mode-toggle">
            <button
              className={`mode-btn ${viewMode === 'table' ? 'active' : ''}`}
              onClick={() => setViewMode('table')}
            >
              <span>☰</span>
              <span>Table</span>
            </button>
            <button
              className={`mode-btn ${viewMode === 'cards' ? 'active' : ''}`}
              onClick={() => setViewMode('cards')}
            >
              <span>▦</span>
              <span>Cards</span>
            </button>
          </div>
          <button className="toolbar-btn" title="Export">
            <span>📋</span>
          </button>
        </div>
      </div>

      <div className="symbol-table-content">
        {viewMode === 'table' ? (
          <div className="table-wrapper">
            <table className="data-table">
              <thead>
                <tr>
                  <th>Name</th>
                  <th>Type</th>
                  <th>Dimensions</th>
                  <th>Size (bytes)</th>
                  <th>Scope</th>
                  <th>Line</th>
                </tr>
              </thead>
              <tbody>
                {filteredSymbols.map((symbol) => (
                  <tr key={symbol.name}>
                    <td>
                      <span className="symbol-name">{symbol.name}</span>
                    </td>
                    <td>
                      <span className="type-badge">{symbol.type}</span>
                    </td>
                    <td>
                      {symbol.size1 ? (
                        <span className="dimension-badge">
                          [{symbol.size1}
                          {symbol.size2 ? `][${symbol.size2}` : ''}]
                        </span>
                      ) : (
                        <span className="no-dimension">scalar</span>
                      )}
                    </td>
                    <td>
                      <span className="size-value">
                        {symbol.size1 && symbol.size2
                          ? symbol.size1 * symbol.size2 * 4
                          : symbol.size1
                          ? symbol.size1 * 4
                          : 4}
                      </span>
                    </td>
                    <td>
                      <span className="scope-badge">
                        {symbol.scope || 'global'}
                      </span>
                    </td>
                    <td>
                      <span className="line-number">
                        {symbol.line || '-'}
                      </span>
                    </td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        ) : (
          <div className="cards-grid">
            {filteredSymbols.map((symbol) => (
              <div key={symbol.name} className="symbol-card">
                <div className="card-header">
                  <span className="card-name">{symbol.name}</span>
                  <span className="card-type-badge">{symbol.type}</span>
                </div>
                <div className="card-body">
                  <div className="card-row">
                    <span className="card-label">Dimensions:</span>
                    <span className="card-value">
                      {symbol.size1 ? (
                        <span className="dimension-badge">
                          [{symbol.size1}
                          {symbol.size2 ? `][${symbol.size2}` : ''}]
                        </span>
                      ) : (
                        <span className="no-dimension">scalar</span>
                      )}
                    </span>
                  </div>
                  <div className="card-row">
                    <span className="card-label">Size:</span>
                    <span className="card-value">
                      {symbol.size1 && symbol.size2
                        ? symbol.size1 * symbol.size2 * 4
                        : symbol.size1
                        ? symbol.size1 * 4
                        : 4}{' '}
                      bytes
                    </span>
                  </div>
                  {symbol.scope && (
                    <div className="card-row">
                      <span className="card-label">Scope:</span>
                      <span className="card-value">{symbol.scope}</span>
                    </div>
                  )}
                </div>
              </div>
            ))}
          </div>
        )}
      </div>
    </div>
  )
}

export default SymbolTableView
