import { useState } from 'react'
import './ASTVisualizer.css'

interface ASTVisualizerProps {
  ast: any
}

interface TreeNodeProps {
  node: any
  depth?: number
}

function TreeNode({ node, depth = 0 }: TreeNodeProps) {
  const [isExpanded, setIsExpanded] = useState(depth < 2)

  if (!node) return null

  const hasChildren = node.children && node.children.length > 0
  const nodeType = node.type || 'Unknown'
  const nodeValue = node.value
  const dataType = node.dataType
  const semanticInfo = node.semanticInfo

  return (
    <div className="tree-node" style={{ marginLeft: `${depth * 20}px` }}>
      <div className="node-header" onClick={() => hasChildren && setIsExpanded(!isExpanded)}>
        {hasChildren && (
          <span className="expand-icon">{isExpanded ? '▼' : '▶'}</span>
        )}
        {!hasChildren && <span className="expand-icon leaf">●</span>}
        
        <span className="node-type">{nodeType}</span>
        
        {nodeValue && (
          <span className="node-value">= {nodeValue}</span>
        )}
        
        {dataType && (
          <span className="node-datatype">{dataType}</span>
        )}
        
        {semanticInfo && (
          <span className="node-semantic">{semanticInfo}</span>
        )}
      </div>

      {hasChildren && isExpanded && (
        <div className="node-children">
          {node.children.map((child: any, index: number) => (
            <TreeNode key={index} node={child} depth={depth + 1} />
          ))}
        </div>
      )}
    </div>
  )
}

function ASTVisualizer({ ast }: ASTVisualizerProps) {
  const [viewMode, setViewMode] = useState<'tree' | 'json'>('tree')

  if (!ast) {
    return <div className="ast-empty">No AST data available</div>
  }

  return (
    <div className="ast-visualizer">
      <div className="ast-controls">
        <button
          className={`view-btn ${viewMode === 'tree' ? 'active' : ''}`}
          onClick={() => setViewMode('tree')}
        >
          Tree View
        </button>
        <button
          className={`view-btn ${viewMode === 'json' ? 'active' : ''}`}
          onClick={() => setViewMode('json')}
        >
          JSON View
        </button>
      </div>

      {viewMode === 'tree' ? (
        <div className="tree-view">
          <TreeNode node={ast} />
        </div>
      ) : (
        <pre className="json-view">{JSON.stringify(ast, null, 2)}</pre>
      )}
    </div>
  )
}

export default ASTVisualizer
