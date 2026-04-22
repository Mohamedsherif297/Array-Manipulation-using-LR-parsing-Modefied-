import { useMemo } from 'react'
import type { CompilerOutput } from '../App'
import './LearnView.css'

interface LearnViewProps {
  output: CompilerOutput | null
}

      type AccessFocus = {
        name: string
        indices: number[]
      }

      function isNode(value: any): value is { type: string; children?: any[]; value?: any } {
        return value && typeof value === 'object' && typeof value.type === 'string'
      }

      function cloneDefaultValue(type: string, size1?: number, size2?: number): any {
        if (size1 && size2) {
          return Array.from({ length: size1 }, () => Array.from({ length: size2 }, () => cloneDefaultValue(type)))
        }
        if (size1) {
          return Array.from({ length: size1 }, () => cloneDefaultValue(type))
        }
        if (type === 'string') return ''
        if (type === 'char') return ''
        return 0
      }

      function flattenArray(value: any): any[] {
        if (!Array.isArray(value)) return [value]
        return value.flatMap((item) => flattenArray(item))
      }

      function collectArrayInit(node: any): any {
        if (!isNode(node)) return null
        if (node.type === 'ArrayInit') {
          return (node.children || [])
            .filter((child) => (isNode(child) ? child.type !== ',' : true))
            .map((child) => collectArrayInit(child))
            .filter((child) => child !== null)
        }
        if (node.type === 'Elements' || node.type === 'Element') {
          return collectArrayInit(node.children?.[0] || node.children?.[node.children.length - 1])
        }
        if (node.type === 'Number' || node.type === 'NUM') return Number(node.value)
        if (node.type === 'STRING') return String(node.value ?? '')
        if (node.type === 'CHAR') return String(node.value ?? '').slice(0, 1)
        if (node.type === 'EndLine') return '\n'
        if (node.children && node.children.length === 1) return collectArrayInit(node.children[0])
        return collectArrayInit(node.value ?? null)
      }

      function evalExpr(node: any, env: Record<string, any>): any {
        if (!isNode(node)) return 0

        if (node.type === 'Number' || node.type === 'NUM') return Number(node.value)
        if (node.type === 'STRING') return String(node.value ?? '')
        if (node.type === 'CHAR') return String(node.value ?? '').slice(0, 1)
        if (node.type === 'EndLine') return '\n'
        if (node.type === 'ID') return env[node.value]

        if (node.type === 'ArrayAccess') {
          const path: any[] = []
          let current = node
          while (current && current.type === 'ArrayAccess') {
            path.push(current.children?.[1])
            current = current.children?.[0]
          }
          if (!current || current.type !== 'ID') return 0
          const name = current.value
          const target = env[name]
          const indices = path.reverse().map((idx) => Number(evalExpr(idx, env)))

          if (typeof target === 'string') {
            const index = indices[0] ?? 0
            return target[index] ?? ''
          }

          if (!Array.isArray(target)) return 0
          if (indices.length === 1) return target[indices[0]]
          if (indices.length >= 2) return target[indices[0]]?.[indices[1]]
          return 0
        }

        if (node.type === '+' || node.type === '-' || node.type === '*' || node.type === '/') {
          const left = evalExpr(node.children?.[0], env)
          const right = evalExpr(node.children?.[1], env)
          if (node.type === '+') return left + right
          if (node.type === '-') return Number(left) - Number(right)
          if (node.type === '*') return Number(left) * Number(right)
          if (node.type === '/') return Number(right) === 0 ? 0 : Number(left) / Number(right)
        }

        if (node.children && node.children.length > 0) {
          return evalExpr(node.children[node.children.length - 1], env)
        }

        return 0
      }

      function assignTarget(target: any, value: any, env: Record<string, any>) {
        if (!isNode(target)) return

        if (target.type === 'ID') {
          env[target.value] = value
          return
        }

        if (target.type !== 'ArrayAccess') return

        const stack: any[] = []
        let current = target
        while (current && current.type === 'ArrayAccess') {
          stack.push(current.children?.[1])
          current = current.children?.[0]
        }
        if (!current || current.type !== 'ID') return

        const name = current.value
        const indices = stack.reverse().map((idx) => Number(evalExpr(idx, env)))
        const currentValue = env[name]

        if (typeof currentValue === 'string') {
          const idx = indices[0] ?? 0
          const replacement = String(value ?? '')
          env[name] = currentValue.slice(0, idx) + replacement + currentValue.slice(Math.min(idx + 1, currentValue.length))
          return
        }

        if (!Array.isArray(currentValue)) return
        if (indices.length === 1) {
          currentValue[indices[0]] = value
          return
        }
        if (indices.length >= 2 && Array.isArray(currentValue[indices[0]])) {
          currentValue[indices[0]][indices[1]] = value
        }
      }

      function buildMemory(output: CompilerOutput | null) {
        const ast = output?.ast
        const symbolTable = output?.symbolTable || {}
        const env: Record<string, any> = {}

        for (const [name, sym] of Object.entries(symbolTable as Record<string, any>)) {
          if (sym?.isArray) {
            env[name] = cloneDefaultValue(sym.type || 'int', sym.size1 || 0, sym.size2 || 0)
          } else {
            env[name] = cloneDefaultValue(sym?.type || 'int')
          }
        }

        const visit = (node: any) => {
          if (!isNode(node)) return

          if (node.type === 'Program' || node.type === 'StmtList' || node.type === 'FunctionDef') {
            for (const child of node.children || []) visit(child)
            return
          }

          if (node.type === 'Declaration') {
            const name = node.children?.[1]?.value
            if (!name) return
            if (!(name in env)) env[name] = cloneDefaultValue(node.children?.[0]?.value || 'int')
            return
          }

          if (node.type === 'DeclAssign') {
            const name = node.children?.[1]?.value
            const rhs = node.children?.[node.children.length - 1]
            if (!name) return
            if (rhs?.type === 'ArrayInit') {
              env[name] = collectArrayInit(rhs)
            } else {
              env[name] = evalExpr(rhs, env)
            }
            return
          }

          if (node.type === 'Assignment') {
            const rhs = evalExpr(node.children?.[1], env)
            assignTarget(node.children?.[0], rhs, env)
            return
          }

          for (const child of node.children || []) visit(child)
        }

        visit(ast)
        return env
      }

      function findLastAccess(ast: any): AccessFocus | null {
        let last: AccessFocus | null = null

        const visit = (node: any) => {
          if (!isNode(node)) return
          if (node.type === 'ArrayAccess') {
            const stack: any[] = []
            let current = node
            while (current && current.type === 'ArrayAccess') {
              stack.push(current.children?.[1])
              current = current.children?.[0]
            }
            if (current?.type === 'ID') {
              const indices = stack.reverse().map((idx) => Number(isNode(idx) ? idx.value : 0))
              if (!indices.some((n) => Number.isNaN(n))) {
                last = { name: current.value, indices }
              }
            }
          }
          for (const child of node.children || []) visit(child)
        }

        visit(ast)
        return last
      }

      function Cell({ value, highlighted = false }: { value: any; highlighted?: boolean }) {
        return <div className={`learn-cell ${highlighted ? 'highlighted' : ''}`}>{String(value ?? '')}</div>
      }

      function LearnView({ output }: LearnViewProps) {
        const memory = useMemo(() => buildMemory(output), [output])
        const accessFocus = useMemo(() => findLastAccess(output?.ast), [output])
        const symbols = Object.entries(output?.symbolTable || {}) as Array<[string, any]>

        if (!output) {
          return (
            <div className="learn-view">
              <div className="learn-header">
                <h2>Variable Visualizer</h2>
                <p>Compile code to inspect variables and memory layout</p>
              </div>
            </div>
          )
        }

        const scalarSymbols = symbols.filter(([, sym]) => !sym?.isArray)
        const arraySymbols = symbols.filter(([, sym]) => sym?.isArray)

        return (
          <div className="learn-view">
            <div className="learn-header">
              <h2>Variable Visualizer</h2>
              <p>Memory snapshot from the latest compilation run</p>
            </div>

            <div className="memory-summary">
              <div className="summary-card">
                <span className="summary-label">Scalars</span>
                <strong>{scalarSymbols.length}</strong>
              </div>
              <div className="summary-card">
                <span className="summary-label">Arrays</span>
                <strong>{arraySymbols.length}</strong>
              </div>
              <div className="summary-card">
                <span className="summary-label">Access Focus</span>
                <strong>{accessFocus ? `${accessFocus.name}[${accessFocus.indices.join('][')}]` : 'None'}</strong>
              </div>
            </div>

            <div className="variable-grid">
              <section className="learn-section">
                <h3>Scalars</h3>
                <div className="scalar-table">
                  <div className="scalar-head">
                    <span>Name</span>
                    <span>Type</span>
                    <span>Value</span>
                  </div>
                  {scalarSymbols.length === 0 ? (
                    <div className="empty-learn">No scalar variables</div>
                  ) : scalarSymbols.map(([name, sym]) => (
                    <div key={name} className="scalar-row">
                      <span className="mono">{name}</span>
                      <span>{sym.type}</span>
                      <span className="mono value-box">{String(memory[name] ?? '')}</span>
                    </div>
                  ))}
                </div>
              </section>

              <section className="learn-section">
                <h3>Arrays</h3>
                {arraySymbols.length === 0 ? (
                  <div className="empty-learn">No array variables</div>
                ) : arraySymbols.map(([name, sym]) => {
                  const value = memory[name]
                  const is2D = sym.size2 > 0
                  const rowCount = Number(sym.size1 || 0)
                  const colCount = Number(sym.size2 || 0)
                  const focus = accessFocus?.name === name ? accessFocus.indices : null

                  return (
                    <div key={name} className="array-card">
                      <div className="array-title">
                        <strong>{name}</strong>
                        <span>{sym.type}{is2D ? `[${rowCount}][${colCount}]` : `[${rowCount}]`}</span>
                      </div>

                      {!is2D ? (
                        <div className="array-1d">
                          <div className="array-index-row">
                            {Array.from({ length: rowCount }, (_, i) => (
                              <div key={`${name}-idx-${i}`} className={`array-index ${focus?.[0] === i ? 'highlighted' : ''}`}>{i}</div>
                            ))}
                          </div>
                          <div className="array-value-row">
                            {Array.from({ length: rowCount }, (_, i) => (
                              <Cell key={`${name}-cell-${i}`} value={Array.isArray(value) ? value[i] : ''} highlighted={focus?.[0] === i} />
                            ))}
                          </div>
                        </div>
                      ) : (
                        <div className="array-2d">
                          <div className="array-col-header">
                            <div className="corner-cell" />
                            {Array.from({ length: colCount }, (_, c) => (
                              <div key={`${name}-col-${c}`} className={`array-index ${focus?.[1] === c ? 'highlighted' : ''}`}>{c}</div>
                            ))}
                          </div>
                          {Array.from({ length: rowCount }, (_, r) => (
                            <div key={`${name}-row-${r}`} className="array-grid-row">
                              <div className={`array-index row-index ${focus?.[0] === r ? 'highlighted' : ''}`}>{r}</div>
                              {Array.from({ length: colCount }, (_, c) => (
                                <Cell
                                  key={`${name}-r${r}-c${c}`}
                                  value={Array.isArray(value?.[r]) ? value[r][c] : ''}
                                  highlighted={focus?.[0] === r && focus?.[1] === c}
                                />
                              ))}
                            </div>
                          ))}
                          <div className="flattened-view">
                            <span className="flattened-label">Flattened</span>
                            <div className="flattened-chips">
                              {flattenArray(value).map((item, i) => (
                                <span key={`${name}-flat-${i}`} className="flatten-chip">{String(item)}</span>
                              ))}
                            </div>
                          </div>
                        </div>
                      )}
                    </div>
                  )
                })}
              </section>
            </div>

            <div className="learn-footer learn-footer-modern">
              <div className="info-box">
                <h3>Current Memory</h3>
                <p>Values are derived from the latest compiler output and updated in memory order.</p>
              </div>
              <div className="info-box">
                <h3>Index Highlighting</h3>
                <p>When the program accesses an array, the latest access path is highlighted in the grid.</p>
              </div>
            </div>
          </div>
        )
      }

      export default LearnView
