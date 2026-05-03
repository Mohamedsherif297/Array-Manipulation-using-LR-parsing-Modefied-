import './ArrayVisualizer.css'

interface ArrayVisualizerProps {
  symbolTable: any
}

function ArrayVisualizer({ symbolTable }: ArrayVisualizerProps) {
  if (!symbolTable) {
    return <div className="array-visualizer-empty">No symbol table available</div>
  }

  const arrays = Object.entries(symbolTable).filter(
    ([_, info]: [string, any]) => info.isArray
  )

  if (arrays.length === 0) {
    return (
      <div className="array-visualizer-empty">
        <p>No arrays declared in the program</p>
      </div>
    )
  }

  return (
    <div className="array-visualizer">
      <div className="array-list">
        {arrays.map(([name, info]: [string, any]) => (
          <div key={name} className="array-card">
            <div className="array-header">
              <h3>{name}</h3>
              <div className="array-meta">
                <span className="array-type">{info.type}</span>
                {info.size2 > 0 ? (
                  <span className="array-dims">
                    [{info.size1}][{info.size2}]
                  </span>
                ) : (
                  <span className="array-dims">[{info.size1}]</span>
                )}
              </div>
            </div>

            <div className="array-visual">
              {info.size2 > 0 ? (
                // 2D Array Visualization
                <div className="array-2d">
                  <div className="array-2d-grid">
                    {Array.from({ length: info.size1 }).map((_, i) => (
                      <div key={i} className="array-row">
                        {Array.from({ length: info.size2 }).map((_, j) => {
                          const flatIndex = i * info.size2 + j
                          return (
                            <div key={j} className="array-cell">
                              <div className="cell-index">[{i}][{j}]</div>
                              <div className="cell-flat">offset: {flatIndex}</div>
                            </div>
                          )
                        })}
                      </div>
                    ))}
                  </div>
                  <div className="array-formula">
                    <strong>Index Formula:</strong> offset = i × {info.size2} + j
                  </div>
                </div>
              ) : (
                // 1D Array Visualization
                <div className="array-1d">
                  <div className="array-1d-grid">
                    {Array.from({ length: info.size1 }).map((_, i) => (
                      <div key={i} className="array-cell">
                        <div className="cell-index">[{i}]</div>
                        <div className="cell-flat">offset: {i}</div>
                      </div>
                    ))}
                  </div>
                  <div className="array-formula">
                    <strong>Index Formula:</strong> offset = i
                  </div>
                </div>
              )}
            </div>

            <div className="array-memory">
              <h4>Flattened Memory Layout</h4>
              <div className="memory-grid">
                {Array.from({ 
                  length: info.size2 > 0 ? info.size1 * info.size2 : info.size1 
                }).map((_, i) => (
                  <div key={i} className="memory-cell">
                    <div className="memory-index">{i}</div>
                    <div className="memory-name">{name}[{i}]</div>
                  </div>
                ))}
              </div>
            </div>
          </div>
        ))}
      </div>
    </div>
  )
}

export default ArrayVisualizer
