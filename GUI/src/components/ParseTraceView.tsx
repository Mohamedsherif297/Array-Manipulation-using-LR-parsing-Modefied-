import { useState, useRef, useEffect } from 'react'
import './ParseTraceView.css'

interface ParseStep {
  stack: string
  input: string
  action: string
}

interface ParseTraceViewProps {
  steps: ParseStep[]
}

export default function ParseTraceView({ steps }: ParseTraceViewProps) {
  const [activeRow, setActiveRow] = useState<number | null>(null)
  const [autoStep, setAutoStep] = useState(false)
  const activeRef = useRef<HTMLTableRowElement>(null)
  const intervalRef = useRef<ReturnType<typeof setInterval> | null>(null)

  // Auto-scroll active row into view
  useEffect(() => {
    activeRef.current?.scrollIntoView({ block: 'nearest', behavior: 'smooth' })
  }, [activeRow])

  // Auto-step mode
  useEffect(() => {
    if (autoStep) {
      intervalRef.current = setInterval(() => {
        setActiveRow(prev => {
          const next = prev === null ? 0 : prev + 1
          if (next >= steps.length) {
            setAutoStep(false)
            return prev
          }
          return next
        })
      }, 600)
    } else {
      if (intervalRef.current) clearInterval(intervalRef.current)
    }
    return () => { if (intervalRef.current) clearInterval(intervalRef.current) }
  }, [autoStep, steps.length])

  if (!steps || steps.length === 0) {
    return (
      <div className="ptv-empty">
        <p>No parse trace available. Compile your code first.</p>
      </div>
    )
  }

  const getActionClass = (action: string) => {
    if (action === 'accept')        return 'action-accept'
    if (action.startsWith('shift')) return 'action-shift'
    if (action.startsWith('reduce'))return 'action-reduce'
    return ''
  }

  const getActionLabel = (action: string) => {
    if (action === 'accept')         return '✓ accept'
    if (action.startsWith('shift'))  return '⇒ ' + action
    if (action.startsWith('reduce')) return '↩ ' + action
    return action
  }

  // Highlight the handle (rightmost symbols being reduced) in the stack
  const renderStack = (stack: string, action: string) => {
    if (!action.startsWith('reduce')) {
      return <span className="ptv-stack-text">{stack}</span>
    }
    // Extract RHS from "reduce LHS → sym1 sym2 ..."
    const arrowIdx = action.indexOf('→')
    if (arrowIdx === -1) return <span className="ptv-stack-text">{stack}</span>
    const rhs = action.slice(arrowIdx + 1).trim()
    const rhsSyms = rhs.split(' ').filter(Boolean)
    // Try to highlight the last N symbols in the stack string
    const stackSyms = stack.split(' ')
    const handleLen = rhsSyms.length
    if (handleLen > 0 && stackSyms.length >= handleLen) {
      const prefix = stackSyms.slice(0, stackSyms.length - handleLen).join(' ')
      const handle = stackSyms.slice(stackSyms.length - handleLen).join(' ')
      return (
        <span className="ptv-stack-text">
          {prefix && <span>{prefix} </span>}
          <span className="ptv-handle">{handle}</span>
        </span>
      )
    }
    return <span className="ptv-stack-text">{stack}</span>
  }

  return (
    <div className="ptv-wrapper">
      {/* Controls */}
      <div className="ptv-toolbar">
        <div className="ptv-toolbar-left">
          <button
            className="ptv-btn"
            onClick={() => setActiveRow(null)}
            disabled={activeRow === null}
          >⏮ Reset</button>
          <button
            className="ptv-btn"
            onClick={() => setActiveRow(r => r === null ? 0 : Math.max(0, r - 1))}
            disabled={activeRow === null || activeRow === 0}
          >◀ Back</button>
          <span className="ptv-counter">
            {activeRow !== null ? `Step ${activeRow + 1} / ${steps.length}` : `${steps.length} steps`}
          </span>
          <button
            className="ptv-btn"
            onClick={() => setActiveRow(r => r === null ? 0 : Math.min(steps.length - 1, r + 1))}
            disabled={activeRow === steps.length - 1}
          >Fwd ▶</button>
          <button
            className="ptv-btn"
            onClick={() => setActiveRow(steps.length - 1)}
            disabled={activeRow === steps.length - 1}
          >⏭ End</button>
        </div>
        <div className="ptv-toolbar-right">
          <button
            className={`ptv-btn ${autoStep ? 'active' : ''}`}
            onClick={() => {
              if (!autoStep && (activeRow === null || activeRow >= steps.length - 1)) {
                setActiveRow(0)
              }
              setAutoStep(a => !a)
            }}
          >{autoStep ? '⏸ Pause' : '▶ Auto'}</button>
        </div>
      </div>

      {/* Legend */}
      <div className="ptv-legend">
        <span className="ptv-legend-item action-shift">shift</span>
        <span className="ptv-legend-item action-reduce">reduce</span>
        <span className="ptv-legend-item action-accept">accept</span>
        <span className="ptv-legend-item ptv-handle-legend">handle (reduced symbols)</span>
      </div>

      {/* Table */}
      <div className="ptv-table-wrapper">
        <table className="ptv-table">
          <thead>
            <tr>
              <th className="ptv-th ptv-th-num">#</th>
              <th className="ptv-th ptv-th-stack">Stack</th>
              <th className="ptv-th ptv-th-input">Input</th>
              <th className="ptv-th ptv-th-action">Action</th>
            </tr>
          </thead>
          <tbody>
            {steps.map((step, i) => {
              const isActive = activeRow === i
              const isPast = activeRow !== null && i < activeRow
              return (
                <tr
                  key={i}
                  ref={isActive ? activeRef : null}
                  className={`ptv-row ${isActive ? 'ptv-row-active' : ''} ${isPast ? 'ptv-row-past' : ''}`}
                  onClick={() => setActiveRow(i)}
                >
                  <td className="ptv-td ptv-td-num">{i + 1}</td>
                  <td className="ptv-td ptv-td-stack">
                    {renderStack(step.stack, step.action)}
                  </td>
                  <td className="ptv-td ptv-td-input">
                    <span className="ptv-input-text">{step.input}</span>
                  </td>
                  <td className={`ptv-td ptv-td-action ${getActionClass(step.action)}`}>
                    {getActionLabel(step.action)}
                  </td>
                </tr>
              )
            })}
          </tbody>
        </table>
      </div>
    </div>
  )
}
