import './SymbolTableView.css'

interface SymbolTableViewProps {
  symbolTable: any
}

function SymbolTableView({ symbolTable }: SymbolTableViewProps) {
  if (!symbolTable || Object.keys(symbolTable).length === 0) {
    return (
      <div className="symbol-table-empty">
        No symbols declared in the program
      </div>
    )
  }

  return (
    <div className="symbol-table-view">
      <div className="table-container">
        <table className="symbol-table">
          <thead>
            <tr>
              <th>Symbol</th>
              <th>Type</th>
              <th>Array</th>
              <th>Dimensions</th>
            </tr>
          </thead>
          <tbody>
            {Object.entries(symbolTable).map(([name, info]: [string, any]) => (
              <tr key={name}>
                <td className="symbol-name">{name}</td>
                <td className="symbol-type">{info.type}</td>
                <td className="symbol-array">
                  {info.isArray ? (
                    <span className="badge badge-yes">Yes</span>
                  ) : (
                    <span className="badge badge-no">No</span>
                  )}
                </td>
                <td className="symbol-dimensions">
                  {info.isArray ? (
                    <>
                      {info.size1 && <span className="dimension">[{info.size1}]</span>}
                      {info.size2 && <span className="dimension">[{info.size2}]</span>}
                    </>
                  ) : (
                    <span className="no-dimension">-</span>
                  )}
                </td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>

      <div className="symbol-details">
        <h3>Symbol Details</h3>
        <pre className="json-display">
          {JSON.stringify(symbolTable, null, 2)}
        </pre>
      </div>
    </div>
  )
}

export default SymbolTableView
