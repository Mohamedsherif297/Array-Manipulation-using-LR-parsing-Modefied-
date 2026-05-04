import { useState } from 'react'
import './TokenStreamView.css'

export interface Token {
  type: string
  lexeme: string
  line: number
}

interface TokenStreamViewProps {
  tokens: Token[]
}

const TOKEN_TYPE_COLORS: Record<string, string> = {
  DATATYPE:    'token-datatype',
  IDENTIFIER:  'token-identifier',
  CONSTANT:    'token-constant',
  STRING:      'token-string',
  CHAR:        'token-char',
  RESERVED:    'token-reserved',
  OPERATOR:    'token-operator',
  SYMBOL:      'token-symbol',
  LBRACKET:    'token-bracket',
  RBRACKET:    'token-bracket',
  LPAREN:      'token-bracket',
  RPAREN:      'token-bracket',
  LBRACE:      'token-bracket',
  RBRACE:      'token-bracket',
  END_OF_FILE: 'token-eof',
  ILLEGAL:     'token-illegal',
}

const ALL_TYPES = [
  'ALL', 'DATATYPE', 'IDENTIFIER', 'CONSTANT', 'STRING', 'CHAR',
  'RESERVED', 'OPERATOR', 'SYMBOL', 'LBRACKET', 'RBRACKET',
  'LPAREN', 'RPAREN', 'LBRACE', 'RBRACE', 'END_OF_FILE',
]

export default function TokenStreamView({ tokens }: TokenStreamViewProps) {
  const [filter, setFilter] = useState('ALL')
  const [search, setSearch] = useState('')
  const [viewMode, setViewMode] = useState<'table' | 'stream'>('table')

  if (!tokens || tokens.length === 0) {
    return (
      <div className="tsv-empty">
        <div className="tsv-empty-icon">🔤</div>
        <p>No token stream available. Compile your code first.</p>
      </div>
    )
  }

  const filtered = tokens.filter(t => {
    const matchType = filter === 'ALL' || t.type === filter
    const matchSearch = search === '' ||
      t.lexeme.toLowerCase().includes(search.toLowerCase()) ||
      t.type.toLowerCase().includes(search.toLowerCase())
    return matchType && matchSearch
  })

  // Count by type for the filter badges
  const countByType = tokens.reduce<Record<string, number>>((acc, t) => {
    acc[t.type] = (acc[t.type] || 0) + 1
    return acc
  }, {})

  return (
    <div className="tsv-wrapper">
      {/* Toolbar */}
      <div className="tsv-toolbar">
        <div className="tsv-toolbar-left">
          <input
            className="tsv-search"
            type="text"
            placeholder="Search lexeme or type…"
            value={search}
            onChange={e => setSearch(e.target.value)}
          />
          <span className="tsv-count">
            {filtered.length} / {tokens.length} tokens
          </span>
        </div>
        <div className="tsv-toolbar-right">
          <button
            className={`tsv-view-btn ${viewMode === 'table' ? 'active' : ''}`}
            onClick={() => setViewMode('table')}
            title="Table view"
          >≡ Table</button>
          <button
            className={`tsv-view-btn ${viewMode === 'stream' ? 'active' : ''}`}
            onClick={() => setViewMode('stream')}
            title="Stream view"
          >◈ Stream</button>
        </div>
      </div>

      {/* Type filter pills */}
      <div className="tsv-filters">
        {ALL_TYPES.map(type => {
          const count = type === 'ALL' ? tokens.length : (countByType[type] || 0)
          if (type !== 'ALL' && count === 0) return null
          return (
            <button
              key={type}
              className={`tsv-filter-pill ${filter === type ? 'active' : ''} ${type !== 'ALL' ? TOKEN_TYPE_COLORS[type] : ''}`}
              onClick={() => setFilter(type)}
            >
              {type}
              <span className="tsv-pill-count">{count}</span>
            </button>
          )
        })}
      </div>

      {/* Content */}
      <div className="tsv-content">
        {viewMode === 'table' ? (
          <div className="tsv-table-wrapper">
            <table className="tsv-table">
              <thead>
                <tr>
                  <th className="tsv-th tsv-th-num">#</th>
                  <th className="tsv-th tsv-th-line">Line</th>
                  <th className="tsv-th tsv-th-type">Type</th>
                  <th className="tsv-th tsv-th-lexeme">Lexeme</th>
                </tr>
              </thead>
              <tbody>
                {filtered.map((token, i) => (
                  <tr key={i} className="tsv-row">
                    <td className="tsv-td tsv-td-num">{i + 1}</td>
                    <td className="tsv-td tsv-td-line">{token.line}</td>
                    <td className="tsv-td tsv-td-type">
                      <span className={`tsv-type-badge ${TOKEN_TYPE_COLORS[token.type] || ''}`}>
                        {token.type}
                      </span>
                    </td>
                    <td className="tsv-td tsv-td-lexeme">
                      <code className="tsv-lexeme">{token.lexeme}</code>
                    </td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        ) : (
          <div className="tsv-stream">
            {filtered.map((token, i) => (
              <span
                key={i}
                className={`tsv-token-chip ${TOKEN_TYPE_COLORS[token.type] || ''}`}
                title={`${token.type} | Line ${token.line}`}
              >
                {token.lexeme}
                <span className="tsv-chip-type">{token.type}</span>
              </span>
            ))}
          </div>
        )}
      </div>
    </div>
  )
}
