import { useState, useEffect } from 'react'
import './ASTView.css'

interface ASTViewProps {
  ast: any
}

interface TreeNodeProps {
  node: any
  level: number
  expanded: Set<string>
  onToggle: (id: string) => void
}

function TreeNode({ node, level, expanded, onToggle }: TreeNodeProps) {
  if (!node) return null

  const nodeId = `${node.type}-${level}-${JSON.stringify(node.value || '')}`
  const isExpanded = expanded.has(nodeId)
  const hasChildren = node.children && Array.isArray(node.children) && node.children.length > 0

  const getNodeIcon = (type: string) => {
    if (type === 'Program' || type === 'FunctionDef') return 'P'
    if (type === 'DeclAssign' || type === 'Decl') return 'D'
    if (type === 'Return') return 'R'
    if (type === 'ArrayAccess') return 'A'
    if (type === 'BinaryOp') return 'O'
    if (type === 'Identifier') return 'I'
    if (type === 'Number' || type === 'Literal') return 'N'
    return 'N'
  }

  const getNodeClass = (type: string) => {
    if (type === 'Program' || type === 'FunctionDef') return 'program'
    if (type === 'DeclAssign' || type === 'Decl') return 'declaration'
    if (type === 'Return') return 'statement'
    if (type === 'BinaryOp' || type === 'ArrayAccess') return 'expression'
    return 'default'
  }

  return (
    <div className="tree-node-container" style={{ marginLeft: `${level * 24}px` }}>
      <div
        className={`node-header ${hasChildren ? 'has-children' : ''}`}
        onClick={() => hasChildren && onToggle(nodeId)}
      >
        {hasChildren && (
          <span className="expand-icon">
            {isExpanded ? '▼' : '▶'}
          </span>
        )}
        {!hasChildren && <span className="expand-icon-spacer" />}
        <span className={`node-icon ${getNodeClass(node.type)}`}>
          {getNodeIcon(node.type)}
        </span>
        <span className="node-label">{node.type}</span>
        {node.value && (
          <span className="node-value">= {node.value}</span>
        )}
      </div>

      {isExpanded && hasChildren && (
        <div className="node-children">
          {node.children.map((child: any, index: number) => {
            const isLast = index === node.children.length - 1
            return (
              <div key={`${nodeId}-child-${index}`} className={`child-wrapper ${isLast ? 'last-child' : ''}`}>
                <TreeNode
                  node={child}
                  level={level + 1}
                  expanded={expanded}
                  onToggle={onToggle}
                />
              </div>
            )
          })}
        </div>
      )}
    </div>
  )
}

function ASTView({ ast }: ASTViewProps) {
  const [expanded, setExpanded] = useState<Set<string>>(new Set())
  const [expandAll, setExpandAll] = useState(true)  // Start with expanded
  const [viewMode, setViewMode] = useState<'graph' | 'json'>('graph')
  const [copied, setCopied] = useState(false)

  // Collect all node IDs recursively
  const collectAllNodeIds = (node: any, level: number, ids: Set<string>) => {
    if (!node) return
    
    const nodeId = `${node.type}-${level}-${JSON.stringify(node.value || '')}`
    ids.add(nodeId)
    
    if (node.children && Array.isArray(node.children)) {
      node.children.forEach((child: any) => {
        collectAllNodeIds(child, level + 1, ids)
      })
    }
  }

  // Auto-expand all nodes on mount
  useEffect(() => {
    if (ast && expandAll && viewMode === 'graph') {
      const allIds = new Set<string>()
      collectAllNodeIds(ast, 0, allIds)
      setExpanded(allIds)
    }
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [ast, viewMode])

  if (!ast) {
    return (
      <div className="ast-empty">
        <p>No AST available. Compile your code first.</p>
      </div>
    )
  }

  const handleToggle = (id: string) => {
    const newExpanded = new Set(expanded)
    if (newExpanded.has(id)) {
      newExpanded.delete(id)
    } else {
      newExpanded.add(id)
    }
    setExpanded(newExpanded)
  }

  const handleExpandAll = () => {
    if (expandAll) {
      // Keep only the root expanded so users still see top-level headers.
      const rootOnly = new Set<string>()
      const rootId = `${ast.type}-0-${JSON.stringify(ast.value || '')}`
      rootOnly.add(rootId)
      setExpanded(rootOnly)
      setExpandAll(false)
    } else {
      const allIds = new Set<string>()
      collectAllNodeIds(ast, 0, allIds)
      setExpanded(allIds)
      setExpandAll(true)
    }
  }

  const handleCopyJson = () => {
    const jsonString = JSON.stringify(ast, null, 2)
    navigator.clipboard.writeText(jsonString)
    setCopied(true)
    setTimeout(() => setCopied(false), 2000)
  }

  // Syntax highlight JSON
  const highlightJson = (json: string) => {
    return json
      .replace(/("(\\u[a-zA-Z0-9]{4}|\\[^u]|[^\\"])*"(\s*:)?)/g, (match) => {
        let cls = 'json-string'
        if (/:$/.test(match)) {
          cls = 'json-key'
        }
        return `<span class="${cls}">${match}</span>`
      })
      .replace(/\b(true|false|null)\b/g, '<span class="json-boolean">$1</span>')
      .replace(/\b(-?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?)\b/g, '<span class="json-number">$1</span>')
  }

  return (
    <div className="ast-view">
      <div className="ast-toolbar">
        <div className="toolbar-section">
          <div className="view-mode-toggle">
            <button
              className={`mode-btn ${viewMode === 'graph' ? 'active' : ''}`}
              onClick={() => setViewMode('graph')}
            >
              <span>Graph</span>
              <span>Graph</span>
            </button>
            <button
              className={`mode-btn ${viewMode === 'json' ? 'active' : ''}`}
              onClick={() => setViewMode('json')}
            >
              <span>{ }</span>
              <span>JSON</span>
            </button>
          </div>
        </div>
        <div className="toolbar-section">
          {viewMode === 'graph' ? (
            <>
              <input
                type="text"
                className="search-box"
                placeholder="Search nodes..."
              />
              <button
                className={`toolbar-btn ${expandAll ? 'active' : ''}`}
                onClick={handleExpandAll}
              >
                <span>{expandAll ? '⬌' : '⬍'}</span>
                <span>{expandAll ? 'Collapse All' : 'Expand All'}</span>
              </button>
            </>
          ) : (
            <button
              className={`toolbar-btn ${copied ? 'copied' : ''}`}
              onClick={handleCopyJson}
              title="Copy JSON to clipboard"
            >
              <span>{copied ? '✓' : 'Copy'}</span>
              <span>{copied ? 'Copied!' : 'Copy JSON'}</span>
            </button>
          )}
        </div>
      </div>

      {viewMode === 'graph' ? (
        <div className="ast-canvas">
          <TreeNode
            node={ast}
            level={0}
            expanded={expanded}
            onToggle={handleToggle}
          />
        </div>
      ) : (
        <div className="ast-json-view">
          <pre 
            className="json-content"
            dangerouslySetInnerHTML={{ 
              __html: highlightJson(JSON.stringify(ast, null, 2)) 
            }}
          />
        </div>
      )}
    </div>
  )
}

export default ASTView
