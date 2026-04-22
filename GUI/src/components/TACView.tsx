import { useState } from 'react'
import './TACView.css'

interface TACViewProps {
  tac?: string
  optimizedTac?: string
  onLineClick: (line: number) => void
}

function TACView({
  tac,
  optimizedTac,
  onLineClick
}: TACViewProps) {
  const [showOptimized, setShowOptimized] = useState(false)

  if (!tac) {
    return (
      <div className="tac-empty">
        <p>No TAC generated yet. Compile your code first.</p>
      </div>
    )
  }

  const displayTac = showOptimized && optimizedTac ? optimizedTac : tac
  const lines = displayTac.split('\n').filter(l => l.trim())

  const highlightTacSyntax = (line: string) => {
    if (!line) return ''
    
    const tokens: Array<{ text: string; type: string }> = []
    let remaining = line
    
    const patterns = [
      { type: 'comment', regex: /^(\/\/.*)/ },
      { type: 'label', regex: /^(L\d+:)/ },
      { type: 'instruction', regex: /^(DECL|ASSIGN|ADD|SUB|MUL|DIV|GOTO|IF|RETURN|CALL|PARAM)\b/ },
      { type: 'temp', regex: /^(t\d+)\b/ },
      { type: 'string', regex: /^("(?:[^"\\]|\\.)*")/ },
      { type: 'array', regex: /^(\[[^\]]+\])/ },
      { type: 'number', regex: /^(\d+)/ },
      { type: 'operator', regex: /^([=+\-*/<>])/ },
      { type: 'identifier', regex: /^([a-zA-Z_]\w*)/ },
      { type: 'whitespace', regex: /^(\s+)/ },
    ]
    
    while (remaining.length > 0) {
      let matched = false
      
      for (const pattern of patterns) {
        const match = remaining.match(pattern.regex)
        if (match) {
          tokens.push({ text: match[1], type: pattern.type })
          remaining = remaining.slice(match[1].length)
          matched = true
          break
        }
      }
      
      if (!matched) {
        tokens.push({ text: remaining[0], type: 'text' })
        remaining = remaining.slice(1)
      }
    }
    
    return tokens.map(token => {
      if (token.type === 'whitespace') return token.text
      if (token.type === 'text') return token.text
      return `<span class="tac-${token.type}">${token.text}</span>`
    }).join('')
  }

  return (
    <div className="tac-view">
      <div className="tac-toolbar">
        <div className="toolbar-section">
          {/* Empty section for future controls */}
        </div>

        <div className="toolbar-section">
          {optimizedTac && (
            <button
              className={`toolbar-btn toggle ${showOptimized ? 'active' : ''}`}
              onClick={() => setShowOptimized(!showOptimized)}
            >
              <span>Show Optimized</span>
            </button>
          )}
          <button className="toolbar-btn" title="Copy TAC">
            <span>📋</span>
          </button>
          <button className="toolbar-btn" title="Export">
            <span>💾</span>
          </button>
        </div>
      </div>

      <div className="tac-content">
        <div className="tac-line-numbers">
          {lines.map((_, index) => (
            <div
              key={index}
              className="tac-line-number"
            >
              {index + 1}
            </div>
          ))}
        </div>

        <div className="tac-code">
          {lines.map((line, index) => (
            <div
              key={index}
              className="tac-line"
              onClick={() => onLineClick(index)}
            >
              <span dangerouslySetInnerHTML={{ __html: highlightTacSyntax(line) }} />
            </div>
          ))}
        </div>
      </div>
    </div>
  )
}

export default TACView
