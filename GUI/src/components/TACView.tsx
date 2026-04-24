import { useState, useEffect, useRef, useMemo } from 'react'
import { optimizeTAC, diffTAC } from '../utils/tacOptimizer'
import './TACView.css'

interface TACViewProps {
  tac?: string
  optimizedTac?: string
  onLineClick: (line: number) => void
  activeStepIndex: number | null
  onStepForward: () => void
  onStepBackward: () => void
  totalSteps: number
}

function TACView({
  tac,
  optimizedTac,
  onLineClick,
  activeStepIndex,
  onStepForward,
  onStepBackward,
  totalSteps
}: TACViewProps) {
  const [showOptimized, setShowOptimized] = useState(false)
  const [showDiff, setShowDiff] = useState(false)
  const activeLineRef = useRef<HTMLDivElement>(null)

  // Compute optimized TAC client-side if backend didn't provide one
  const computedOptimized = useMemo(() => tac ? optimizeTAC(tac) : '', [tac])
  const effectiveOptimized = optimizedTac || computedOptimized
  const diff = useMemo(() => tac ? diffTAC(tac, effectiveOptimized) : [], [tac, effectiveOptimized])

  if (!tac) {
    return (
      <div className="tac-empty">
        <p>No TAC generated yet. Compile your code first.</p>
      </div>
    )
  }

  const displayTac = showOptimized ? effectiveOptimized : tac
  const lines = (displayTac || '').split('\n').filter(l => l.trim())

  // Auto-scroll active line into view
  useEffect(() => {
    activeLineRef.current?.scrollIntoView({ block: 'nearest', behavior: 'smooth' })
  }, [activeStepIndex])

  // Strip the ; line:N annotation before display
  const stripAnnotation = (line: string) => line.replace(/\s*;\s*line:\d+\s*$/, '')

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
          <button
            className="toolbar-btn"
            onClick={onStepBackward}
            disabled={activeStepIndex === null || activeStepIndex === 0}
            title="Step backward"
          >◀ Back</button>
          <span className="step-indicator">
            {activeStepIndex !== null ? `${activeStepIndex + 1} / ${totalSteps}` : `— / ${totalSteps}`}
          </span>
          <button
            className="toolbar-btn"
            onClick={onStepForward}
            disabled={totalSteps === 0 || activeStepIndex === totalSteps - 1}
            title="Step forward"
          >Fwd ▶</button>
        </div>

        <div className="toolbar-section">
          <button
            className={`toolbar-btn toggle ${showDiff ? 'active' : ''}`}
            onClick={() => { setShowDiff(d => !d); setShowOptimized(false) }}
          >
            ⚡ Diff
          </button>
          <button
            className={`toolbar-btn toggle ${showOptimized && !showDiff ? 'active' : ''}`}
            onClick={() => { setShowOptimized(o => !o); setShowDiff(false) }}
          >
            ✨ Optimized
          </button>
          <button className="toolbar-btn" title="Copy TAC">
            <span>📋</span>
          </button>
          <button className="toolbar-btn" title="Export">
            <span>💾</span>
          </button>
        </div>
      </div>

      {showDiff ? (
        /* ── DIFF VIEW ── */
        <div className="tac-diff-view">
          <div className="tac-diff-header">
            <div className="tac-diff-col-header tac-diff-original">Original TAC</div>
            <div className="tac-diff-col-header tac-diff-optimized">Optimized TAC</div>
          </div>
          <div className="tac-diff-body">
            {diff.map((d, i) => (
              <div key={i} className={`tac-diff-row tac-diff-${d.status}`}>
                <div className="tac-diff-cell tac-diff-left">
                  {d.original !== null ? (
                    <span dangerouslySetInnerHTML={{ __html: highlightTacSyntax(d.original) }} />
                  ) : <span className="tac-diff-empty">—</span>}
                </div>
                <div className="tac-diff-cell tac-diff-right">
                  {d.optimized !== null ? (
                    <span dangerouslySetInnerHTML={{ __html: highlightTacSyntax(d.optimized) }} />
                  ) : <span className="tac-diff-empty">—</span>}
                </div>
              </div>
            ))}
          </div>
          <div className="tac-diff-legend">
            <span className="diff-badge diff-same">unchanged</span>
            <span className="diff-badge diff-folded">folded</span>
            <span className="diff-badge diff-removed">removed</span>
            <span className="diff-badge diff-added">added</span>
          </div>
        </div>
      ) : (
        /* ── NORMAL / OPTIMIZED VIEW ── */
        <div className="tac-content">
          <div className="tac-line-numbers">
            {lines.map((_, index) => (
              <div
                key={index}
                className={`tac-line-number ${activeStepIndex === index ? 'active' : ''}`}
              >
                {index + 1}
              </div>
            ))}
          </div>

          <div className="tac-code">
            {lines.map((line, index) => (
              <div
                key={index}
                ref={activeStepIndex === index ? activeLineRef : null}
                className={`tac-line ${activeStepIndex === index ? 'active' : ''}`}
                onClick={() => onLineClick(index)}
              >
                <span dangerouslySetInnerHTML={{ __html: highlightTacSyntax(stripAnnotation(line)) }} />
              </div>
            ))}
          </div>
        </div>
      )}
    </div>
  )
}

export default TACView
