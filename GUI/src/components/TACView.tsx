import './TACView.css'

interface TACViewProps {
  tac?: string
}

function TACView({ tac }: TACViewProps) {
  if (!tac) {
    return (
      <div className="tac-empty">
        No Three-Address Code generated
      </div>
    )
  }

  const lines = tac.split('\n').filter(line => line.trim())

  return (
    <div className="tac-view">
      <div className="tac-header">
        <h3>Intermediate Representation (TAC)</h3>
        <div className="tac-stats">
          <span className="stat">
            <strong>{lines.length}</strong> instructions
          </span>
        </div>
      </div>

      <div className="tac-code">
        {lines.map((line, index) => {
          const isComment = line.trim().startsWith('//')
          const isAssignment = line.includes('=') && !isComment
          const isArrayAccess = line.includes('[') && line.includes(']')

          return (
            <div key={index} className="tac-line">
              <span className="line-number">{index + 1}</span>
              <span className={`line-content ${isComment ? 'comment' : ''}`}>
                {isComment ? (
                  <span className="tac-comment">{line}</span>
                ) : isAssignment ? (
                  formatAssignment(line)
                ) : (
                  line
                )}
              </span>
            </div>
          )
        })}
      </div>

      <div className="tac-raw">
        <h4>Raw Output</h4>
        <pre>{tac}</pre>
      </div>
    </div>
  )
}

function formatAssignment(line: string) {
  const parts = line.split('=').map(p => p.trim())
  
  if (parts.length !== 2) return <span>{line}</span>

  const [left, right] = parts

  return (
    <>
      <span className="tac-variable">{left}</span>
      <span className="tac-operator"> = </span>
      <span className="tac-expression">{right}</span>
    </>
  )
}

export default TACView
