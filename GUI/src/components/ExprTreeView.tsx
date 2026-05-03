import { useState, useMemo } from 'react'
import './ExprTreeView.css'

interface ExprTreeViewProps {
  ast: any
}

// Node types that are expressions (not statements)
const EXPR_TYPES = new Set([
  '+', '-', '*', '/', 'ID', 'Number', 'NUM', 'STRING', 'CHAR',
  'ArrayAccess', 'BinaryOp', 'Expr', 'Term', 'Factor'
])

const STMT_TYPES = new Set([
  'DeclAssign', 'Declaration', 'Assignment', 'Output', 'Input',
  'Return', 'FunctionDef', 'StmtList', 'Program'
])

// Collect all expression-rooted subtrees from the AST
function collectExpressions(node: any, parentLabel: string = ''): Array<{ label: string; node: any }> {
  if (!node || typeof node !== 'object') return []
  const results: Array<{ label: string; node: any }> = []

  // If this is a binary op or array access — it's an interesting expression
  if (['+', '-', '*', '/'].includes(node.type) && node.children?.length >= 2) {
    const left = node.children[0]?.value || node.children[0]?.type || '?'
    const right = node.children[1]?.value || node.children[1]?.type || '?'
    results.push({
      label: `${left} ${node.type} ${right}`,
      node
    })
  }

  if (node.type === 'ArrayAccess') {
    // Walk to base name
    let base = node
    while (base?.type === 'ArrayAccess') base = base.children?.[0]
    const name = base?.value || '?'
    results.push({ label: `${name}[...]`, node })
  }

  // Recurse into children
  for (const child of node.children || []) {
    results.push(...collectExpressions(child, node.type))
  }

  return results
}

// Operator precedence label
function opLabel(type: string): string {
  if (type === '+') return '+'
  if (type === '-') return '−'
  if (type === '*') return '×'
  if (type === '/') return '÷'
  return type
}

// Render a tree node recursively
function TreeNode({ node, depth = 0 }: { node: any; depth?: number }) {
  const [collapsed, setCollapsed] = useState(false)
  if (!node) return null

  const hasChildren = node.children && node.children.length > 0
  const isBinaryOp = ['+', '-', '*', '/'].includes(node.type)
  const isArrayAccess = node.type === 'ArrayAccess'
  const isLeaf = !hasChildren || STMT_TYPES.has(node.type)

  // Determine display label
  let label = ''
  let nodeClass = 'tree-node-default'

  if (isBinaryOp) {
    label = opLabel(node.type)
    nodeClass = 'tree-node-op'
  } else if (node.type === 'ArrayAccess') {
    label = '[ ]'
    nodeClass = 'tree-node-access'
  } else if (node.type === 'ID') {
    label = node.value || 'ID'
    nodeClass = 'tree-node-id'
  } else if (node.type === 'Number' || node.type === 'NUM') {
    label = node.value || '0'
    nodeClass = 'tree-node-num'
  } else if (node.type === 'STRING') {
    label = `"${node.value}"`
    nodeClass = 'tree-node-str'
  } else if (node.type === 'CHAR') {
    label = `'${node.value}'`
    nodeClass = 'tree-node-str'
  } else {
    label = node.value ? `${node.type}(${node.value})` : node.type
  }

  // Only render expression-relevant nodes
  if (STMT_TYPES.has(node.type)) return null

  const visibleChildren = (node.children || []).filter(
    (c: any) => c && !STMT_TYPES.has(c.type)
  )

  return (
    <div className="tree-node-wrapper" style={{ paddingLeft: depth === 0 ? 0 : 20 }}>
      <div
        className={`tree-node ${nodeClass} ${hasChildren && !isLeaf ? 'has-children' : ''}`}
        onClick={() => hasChildren && setCollapsed(c => !c)}
        title={node.type + (node.value ? ` = ${node.value}` : '')}
      >
        {visibleChildren.length > 0 && (
          <span className="tree-toggle">{collapsed ? '▶' : '▼'}</span>
        )}
        <span className="tree-label">{label}</span>
        {node.line ? <span className="tree-line-badge">L{node.line}</span> : null}
      </div>

      {!collapsed && visibleChildren.map((child: any, i: number) => (
        <div key={i} className="tree-branch">
          <div className="tree-connector" />
          <TreeNode node={child} depth={depth + 1} />
        </div>
      ))}
    </div>
  )
}

export default function ExprTreeView({ ast }: ExprTreeViewProps) {
  const expressions = useMemo(() => collectExpressions(ast), [ast])
  const [selectedIdx, setSelectedIdx] = useState<number | null>(null)

  if (!ast) {
    return (
      <div className="expr-tree-empty">
        <p>Compile your code to explore expression trees.</p>
      </div>
    )
  }

  if (expressions.length === 0) {
    return (
      <div className="expr-tree-empty">
        <p>No expressions found in the current AST.</p>
      </div>
    )
  }

  const selected = selectedIdx !== null ? expressions[selectedIdx] : null

  return (
    <div className="expr-tree-view">
      <div className="expr-tree-sidebar">
        <div className="expr-tree-sidebar-header">Expressions</div>
        {expressions.map((expr, i) => (
          <div
            key={i}
            className={`expr-list-item ${selectedIdx === i ? 'active' : ''}`}
            onClick={() => setSelectedIdx(i)}
          >
            <span className="expr-list-label">{expr.label}</span>
            {expr.node.line && (
              <span className="expr-list-line">L{expr.node.line}</span>
            )}
          </div>
        ))}
      </div>

      <div className="expr-tree-main">
        {selected ? (
          <>
            <div className="expr-tree-title">
              <span>Expression Tree</span>
              <span className="expr-tree-subtitle">{selected.label}</span>
            </div>
            <div className="expr-tree-canvas">
              <TreeNode node={selected.node} depth={0} />
            </div>
            <div className="expr-tree-legend">
              <span className="legend-item tree-node-op">Operator</span>
              <span className="legend-item tree-node-id">Variable</span>
              <span className="legend-item tree-node-num">Number</span>
              <span className="legend-item tree-node-access">Array Access</span>
              <span className="legend-item tree-node-str">String/Char</span>
            </div>
          </>
        ) : (
          <div className="expr-tree-placeholder">
            <p>← Select an expression to view its tree</p>
          </div>
        )}
      </div>
    </div>
  )
}
