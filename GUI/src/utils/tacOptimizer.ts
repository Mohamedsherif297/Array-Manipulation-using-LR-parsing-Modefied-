// tacOptimizer.ts
<<<<<<< HEAD
// Client-side TAC optimizer: constant folding + dead code elimination.
// Operates on the raw TAC string, returns optimized TAC string.
// Does NOT touch the backend or AST.

export interface TACDiffLine {
  original: string | null   // null = line was added
  optimized: string | null  // null = line was removed
  status: 'same' | 'removed' | 'added' | 'folded'
}

// -----------------------------------------------------------------------
// Parse a TAC line into its parts
// -----------------------------------------------------------------------
interface TACLine {
  raw: string
  clean: string          // without ; line:N annotation
  annotation: string     // the ; line:N part
  result?: string        // LHS variable
  op?: string
  arg1?: string
  arg2?: string
  isComment: boolean
  isStore: boolean       // arr[i] = val
  isPrint: boolean
  isRead: boolean
}

=======
// Client-side TAC optimizer — 4 passes matching Lecture 5 (Code Optimization):
//   1. Constant Folding
//   2. Constant Propagation
//   3. Common Subexpression Elimination (CSE)
//   4. Dead Code Elimination
//
// Works as a pure post-processing step on the TAC string.
// Does NOT touch parser, AST, or TAC generation.

// ─────────────────────────────────────────────────────────────────────────────
// Types
// ─────────────────────────────────────────────────────────────────────────────

export interface TACDiffLine {
  original:  string | null
  optimized: string | null
  status: 'same' | 'removed' | 'folded' | 'propagated' | 'cse' | 'added'
}

// Internal representation of one parsed TAC instruction
interface TACLine {
  raw:        string
  clean:      string    // without ; line:N annotation
  annotation: string    // the ; line:N part (preserved in output)
  result?:    string    // LHS variable / destination
  op?:        string    // operator or instruction type
  arg1?:      string
  arg2?:      string
  isComment:  boolean
  isStore:    boolean   // arr[idx] = val
  isPrint:    boolean   // PRINT val
  isPrintln:  boolean   // PRINTLN
  isRead:     boolean   // READ var
  isDecl:     boolean   // // DECL ...
  // Optimization tags (set during passes)
  _tag?: 'folded' | 'propagated' | 'cse' | 'dead'
}

// ─────────────────────────────────────────────────────────────────────────────
// Parser
// ─────────────────────────────────────────────────────────────────────────────

>>>>>>> karim-radwan
function parseLine(raw: string): TACLine {
  const annotation = raw.match(/(\s*;\s*line:\d+\s*)$/)?.[1] ?? ''
  const clean = raw.replace(/\s*;\s*line:\d+\s*$/, '').trim()

<<<<<<< HEAD
  const base: TACLine = { raw, clean, annotation, isComment: false, isStore: false, isPrint: false, isRead: false }

  if (clean.startsWith('//')) { base.isComment = true; return base }

  // PRINT / READ
  const printM = clean.match(/^PRINT\s+(.+)$/)
  if (printM) { base.isPrint = true; base.arg1 = printM[1].trim(); return base }
  const readM = clean.match(/^READ\s+(.+)$/)
  if (readM) { base.isRead = true; base.result = readM[1].trim(); return base }

  // arr[i] = val  (STORE)
  const storeM = clean.match(/^([a-zA-Z_]\w*)\[(.+?)\]\s*=\s*(.+)$/)
  if (storeM) {
    base.isStore = true
    base.arg1 = storeM[1]   // array name
    base.arg2 = storeM[2]   // index
    base.result = storeM[3] // value
    return base
  }

  // result = arg1 op arg2
  const binM = clean.match(/^([a-zA-Z_]\w*)\s*=\s*(.+?)\s*([+\-*\/])\s*(.+)$/)
  if (binM) {
    base.result = binM[1]
    base.arg1 = binM[2].trim()
    base.op = binM[3]
    base.arg2 = binM[4].trim()
=======
  const base: TACLine = {
    raw, clean, annotation,
    isComment: false, isStore: false, isPrint: false,
    isPrintln: false, isRead: false, isDecl: false
  }

  if (clean.startsWith('//')) {
    base.isComment = true
    base.isDecl = clean.startsWith('// DECL')
    return base
  }

  if (clean === 'PRINTLN') { base.isPrintln = true; return base }

  const printM = clean.match(/^PRINT\s+(.+)$/)
  if (printM) { base.isPrint = true; base.arg1 = printM[1].trim(); return base }

  const readM = clean.match(/^READ\s+(.+)$/)
  if (readM) { base.isRead = true; base.result = readM[1].trim(); return base }

  // arr[idx] = val  (STORE)
  const storeM = clean.match(/^([a-zA-Z_]\w*)\[(.+?)\]\s*=\s*(.+)$/)
  if (storeM) {
    base.isStore = true
    base.arg1   = storeM[1]   // array name
    base.arg2   = storeM[2]   // index expression
    base.result = storeM[3].trim() // value being stored
>>>>>>> karim-radwan
    return base
  }

  // result = arg1[arg2]  (LOAD)
  const loadM = clean.match(/^([a-zA-Z_]\w*)\s*=\s*([a-zA-Z_]\w*)\[(.+?)\]$/)
  if (loadM) {
    base.result = loadM[1]
<<<<<<< HEAD
    base.arg1 = loadM[2]
    base.arg2 = loadM[3]
    base.op = 'LOAD'
=======
    base.op     = 'LOAD'
    base.arg1   = loadM[2]
    base.arg2   = loadM[3]
    return base
  }

  // result = arg1 op arg2  (binary)
  const binM = clean.match(/^([a-zA-Z_]\w*)\s*=\s*(.+?)\s*([+\-*\/])\s*(.+)$/)
  if (binM) {
    base.result = binM[1]
    base.arg1   = binM[2].trim()
    base.op     = binM[3]
    base.arg2   = binM[4].trim()
>>>>>>> karim-radwan
    return base
  }

  // result = arg1  (ASSIGN)
  const assignM = clean.match(/^([a-zA-Z_]\w*)\s*=\s*(.+)$/)
  if (assignM) {
    base.result = assignM[1]
<<<<<<< HEAD
    base.arg1 = assignM[2].trim()
=======
    base.arg1   = assignM[2].trim()
>>>>>>> karim-radwan
    return base
  }

  return base
}

<<<<<<< HEAD
function isNumeric(s: string): boolean {
  return /^-?\d+(\.\d+)?$/.test(s.trim())
=======
// Rebuild the clean string from parsed fields
function rebuildClean(line: TACLine): string {
  if (line.isComment || line.isPrintln || line.isPrint || line.isRead) return line.clean
  if (line.isStore) return `${line.arg1}[${line.arg2}] = ${line.result}`
  if (line.op === 'LOAD') return `${line.result} = ${line.arg1}[${line.arg2}]`
  if (line.op && line.arg2 !== undefined) return `${line.result} = ${line.arg1} ${line.op} ${line.arg2}`
  if (line.result && line.arg1 !== undefined) return `${line.result} = ${line.arg1}`
  return line.clean
}

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

function isNumeric(s: string): boolean {
  return /^-?\d+(\.\d+)?$/.test((s ?? '').trim())
}

function isTempVar(s: string): boolean {
  return /^t\d+$/.test(s ?? '')
>>>>>>> karim-radwan
}

function foldBinary(a: string, op: string, b: string): string | null {
  if (!isNumeric(a) || !isNumeric(b)) return null
  const na = parseFloat(a), nb = parseFloat(b)
  let result: number
<<<<<<< HEAD
  if (op === '+') result = na + nb
=======
  if      (op === '+') result = na + nb
>>>>>>> karim-radwan
  else if (op === '-') result = na - nb
  else if (op === '*') result = na * nb
  else if (op === '/') { if (nb === 0) return null; result = na / nb }
  else return null
<<<<<<< HEAD
  // Keep as integer if both operands were integers and result is whole
  if (!a.includes('.') && !b.includes('.') && Number.isInteger(result)) {
    return String(result)
  }
  return String(result)
}

// -----------------------------------------------------------------------
// Main optimizer
// -----------------------------------------------------------------------
export function optimizeTAC(tac: string): string {
  const rawLines = tac.split('\n').filter(l => l.trim())
  const parsed = rawLines.map(parseLine)

  // Pass 1: Constant folding
  // Build a map of temp → constant value as we go
  const constMap = new Map<string, string>()

  const folded = parsed.map(line => {
    if (line.isComment || line.isStore || line.isPrint || line.isRead) return line

    // Substitute known constants into args
    const resolve = (v: string) => constMap.get(v) ?? v

    if (line.op && line.arg1 !== undefined && line.arg2 !== undefined && line.op !== 'LOAD') {
      const a = resolve(line.arg1)
      const b = resolve(line.arg2)
      const folded = foldBinary(a, line.op, b)
      if (folded !== null && line.result) {
        constMap.set(line.result, folded)
        // Rewrite line as simple assignment
        const newClean = `${line.result} = ${folded}`
        return { ...line, clean: newClean, arg1: folded, arg2: undefined, op: undefined, _folded: true }
      }
      // Even if not fully foldable, substitute constants
      if (a !== line.arg1 || b !== line.arg2) {
        const newClean = `${line.result} = ${a} ${line.op} ${b}`
        return { ...line, clean: newClean, arg1: a, arg2: b }
      }
    }

    // Simple assign: result = constant
    if (!line.op && line.arg1 !== undefined && line.result) {
      const a = resolve(line.arg1)
      if (isNumeric(a)) {
        constMap.set(line.result, a)
        if (a !== line.arg1) {
          return { ...line, clean: `${line.result} = ${a}`, arg1: a }
=======
  if (!a.includes('.') && !b.includes('.') && Number.isInteger(result)) return String(result)
  return String(result)
}

// ─────────────────────────────────────────────────────────────────────────────
// Pass 1 — Constant Folding
// Evaluate constant expressions at compile time.
// t1 = 2 + 3  →  t1 = 5
// ─────────────────────────────────────────────────────────────────────────────
function passConstantFolding(lines: TACLine[]): TACLine[] {
  return lines.map(line => {
    if (line.isComment || line.isStore || line.isPrint || line.isRead || line.isPrintln) return line
    if (!line.op || line.op === 'LOAD' || !line.arg1 || !line.arg2) return line

    const folded = foldBinary(line.arg1, line.op, line.arg2)
    if (folded === null) return line

    return {
      ...line,
      clean:  `${line.result} = ${folded}`,
      arg1:   folded,
      arg2:   undefined,
      op:     undefined,
      _tag:   'folded' as const
    }
  })
}

// ─────────────────────────────────────────────────────────────────────────────
// Pass 2 — Constant Propagation
// Replace variables with their known constant values.
// a = 5; b = a + 3  →  b = 5 + 3  →  (folding) b = 8
// ─────────────────────────────────────────────────────────────────────────────
function passConstantPropagation(lines: TACLine[]): TACLine[] {
  const constMap = new Map<string, string>() // var → constant value

  return lines.map(line => {
    if (line.isComment || line.isPrintln) return line

    const resolve = (v: string) => constMap.get(v) ?? v

    // Propagate into PRINT arg
    if (line.isPrint && line.arg1) {
      const a = resolve(line.arg1)
      if (a !== line.arg1) {
        constMap.set(line.arg1, a) // not needed but harmless
        return { ...line, clean: `PRINT ${a}`, arg1: a, _tag: 'propagated' as const }
      }
      return line
    }

    // Propagate into STORE value
    if (line.isStore && line.result) {
      const v = resolve(line.result)
      if (v !== line.result) {
        const newClean = `${line.arg1}[${line.arg2}] = ${v}`
        return { ...line, clean: newClean, result: v, _tag: 'propagated' as const }
      }
      return line
    }

    // Propagate into binary / assign
    let changed = false
    let a1 = line.arg1 ?? ''
    let a2 = line.arg2 ?? ''

    if (a1 && !isNumeric(a1)) {
      const r = resolve(a1)
      if (r !== a1) { a1 = r; changed = true }
    }
    if (a2 && !isNumeric(a2)) {
      const r = resolve(a2)
      if (r !== a2) { a2 = r; changed = true }
    }

    // After propagation, try to fold again
    let newLine: TACLine = changed
      ? { ...line, arg1: a1, arg2: a2 || undefined, _tag: 'propagated' as const }
      : line

    if (changed && newLine.op && newLine.arg2 !== undefined) {
      const folded = foldBinary(a1, newLine.op, a2)
      if (folded !== null) {
        newLine = {
          ...newLine,
          arg1: folded, arg2: undefined, op: undefined,
          _tag: 'propagated' as const
        }
        if (newLine.result) constMap.set(newLine.result, folded)
      }
    }

    newLine.clean = rebuildClean(newLine)

    // Track new constant assignments
    if (!newLine.op && newLine.result && newLine.arg1 && isNumeric(newLine.arg1)) {
      constMap.set(newLine.result, newLine.arg1)
    }
    // Invalidate if reassigned with non-constant
    if (newLine.result && !isNumeric(newLine.arg1 ?? '')) {
      if (!newLine.op) constMap.delete(newLine.result)
    }

    return newLine
  })
}

// ─────────────────────────────────────────────────────────────────────────────
// Pass 3 — Common Subexpression Elimination (CSE)
// Avoid recomputing identical expressions.
// t1 = a + b; t2 = a + b  →  t2 = t1
// ─────────────────────────────────────────────────────────────────────────────
function passCSE(lines: TACLine[]): TACLine[] {
  // expr key → temp that already holds the result
  const exprMap = new Map<string, string>()

  return lines.map(line => {
    if (line.isComment || line.isStore || line.isPrint || line.isRead || line.isPrintln) return line
    if (!line.op || line.op === 'LOAD' || !line.arg1 || !line.arg2) {
      // Invalidate CSE map when a variable is assigned
      if (line.result && !line.op) {
        for (const [key] of exprMap) {
          if (key.includes(line.result)) exprMap.delete(key)
        }
      }
      return line
    }

    const key = `${line.arg1} ${line.op} ${line.arg2}`

    if (exprMap.has(key)) {
      // Replace with assignment from the previously computed temp
      const prev = exprMap.get(key)!
      return {
        ...line,
        clean: `${line.result} = ${prev}`,
        op:    undefined,
        arg1:  prev,
        arg2:  undefined,
        _tag:  'cse' as const
      }
    }

    // Record this expression
    if (line.result) exprMap.set(key, line.result)

    // Invalidate entries that use the result variable
    if (line.result) {
      for (const [key] of exprMap) {
        const parts = key.split(' ')
        if (parts[0] === line.result || parts[2] === line.result) {
          exprMap.delete(key)
>>>>>>> karim-radwan
        }
      }
    }

    return line
  })
<<<<<<< HEAD

  // Pass 2: Dead code elimination
  // A temp variable (tN) that is never read after being written can be removed.
  // We only eliminate pure temp assignments (not stores, prints, reads, or named vars).
  const isTempVar = (s: string) => /^t\d+$/.test(s)

  // Count reads of each temp
  const readCount = new Map<string, number>()
  for (const line of folded) {
    const refs = [line.arg1, line.arg2, line.result].filter(Boolean) as string[]
    // For binary ops and loads, arg1/arg2 are reads; result is a write
    if (line.op || (!line.op && line.arg1)) {
      if (line.arg1) {
        const cur = readCount.get(line.arg1) ?? 0
        readCount.set(line.arg1, cur + 1)
      }
      if (line.arg2) {
        const cur = readCount.get(line.arg2) ?? 0
        readCount.set(line.arg2, cur + 1)
      }
    }
    if (line.isPrint && line.arg1) {
      readCount.set(line.arg1, (readCount.get(line.arg1) ?? 0) + 1)
    }
    if (line.isStore) {
      // result is the value being stored — it's a read
      if (line.result) readCount.set(line.result, (readCount.get(line.result) ?? 0) + 1)
      if (line.arg2) readCount.set(line.arg2, (readCount.get(line.arg2) ?? 0) + 1)
    }
  }

  const optimized = folded.filter(line => {
    if (line.isComment || line.isStore || line.isPrint || line.isRead) return true
    // Remove temp assignments that are never read
    if (line.result && isTempVar(line.result)) {
      const reads = readCount.get(line.result) ?? 0
      if (reads === 0) return false
    }
    return true
  })

  return optimized.map(l => l.annotation ? l.clean + l.annotation : l.clean).join('\n')
}

// -----------------------------------------------------------------------
// Diff: compare original vs optimized TAC lines
// -----------------------------------------------------------------------
export function diffTAC(original: string, optimized: string): TACDiffLine[] {
  const origLines = original.split('\n').filter(l => l.trim()).map(l => l.replace(/\s*;\s*line:\d+\s*$/, '').trim())
  const optLines  = optimized.split('\n').filter(l => l.trim()).map(l => l.replace(/\s*;\s*line:\d+\s*$/, '').trim())
=======
}

// ─────────────────────────────────────────────────────────────────────────────
// Pass 4 — Dead Code Elimination
// Remove temp assignments whose result is never read.
// t1 = a + b  (t1 never used)  →  removed
// ─────────────────────────────────────────────────────────────────────────────
function passDeadCode(lines: TACLine[]): TACLine[] {
  // Count how many times each variable is read
  const readCount = new Map<string, number>()

  const bump = (v?: string) => {
    if (!v) return
    readCount.set(v, (readCount.get(v) ?? 0) + 1)
  }

  for (const line of lines) {
    if (line.isComment || line.isDecl) continue

    if (line.isPrint)   { bump(line.arg1); continue }
    if (line.isRead)    { continue }
    if (line.isPrintln) { continue }

    if (line.isStore) {
      bump(line.arg1)   // array name is read
      bump(line.arg2)   // index is read
      bump(line.result) // value being stored is read
      continue
    }

    // For all other instructions, arg1 and arg2 are reads
    if (line.op === 'LOAD') { bump(line.arg1); bump(line.arg2); continue }
    bump(line.arg1)
    bump(line.arg2)
  }

  return lines.map(line => {
    if (line.isComment || line.isStore || line.isPrint || line.isRead || line.isPrintln) return line
    if (!line.result || !isTempVar(line.result)) return line
    if ((readCount.get(line.result) ?? 0) === 0) {
      return { ...line, _tag: 'dead' as const }
    }
    return line
  })
}

// ─────────────────────────────────────────────────────────────────────────────
// Main entry point
// ─────────────────────────────────────────────────────────────────────────────
export function optimizeTAC(tac: string): string {
  const rawLines = tac.split('\n').filter(l => l.trim())
  let lines = rawLines.map(parseLine)

  // Apply passes in order
  lines = passConstantFolding(lines)
  lines = passConstantPropagation(lines)
  lines = passCSE(lines)
  lines = passDeadCode(lines)

  // Remove dead lines, keep everything else
  return lines
    .filter(l => l._tag !== 'dead')
    .map(l => l.annotation ? l.clean + l.annotation : l.clean)
    .join('\n')
}

// ─────────────────────────────────────────────────────────────────────────────
// Diff — compare original vs optimized with per-line status
// ─────────────────────────────────────────────────────────────────────────────
export function diffTAC(original: string, optimized: string): TACDiffLine[] {
  const strip = (s: string) => s.replace(/\s*;\s*line:\d+\s*$/, '').trim()

  const origLines = original.split('\n').filter(l => l.trim()).map(strip)
  const optLines  = optimized.split('\n').filter(l => l.trim()).map(strip)

  // Build a tagged map from the optimizer for status lookup
  const rawLines = original.split('\n').filter(l => l.trim())
  let tagged = rawLines.map(parseLine)
  tagged = passConstantFolding(tagged)
  tagged = passConstantPropagation(tagged)
  tagged = passCSE(tagged)
  tagged = passDeadCode(tagged)

  // Map original clean line → tag
  const tagMap = new Map<string, TACLine['_tag']>()
  for (const t of tagged) {
    if (t._tag) tagMap.set(strip(t.raw), t._tag)
  }
>>>>>>> karim-radwan

  const result: TACDiffLine[] = []
  let i = 0, j = 0

  while (i < origLines.length || j < optLines.length) {
    const o = origLines[i]
    const n = optLines[j]

    if (i >= origLines.length) {
      result.push({ original: null, optimized: n, status: 'added' }); j++; continue
    }
    if (j >= optLines.length) {
<<<<<<< HEAD
=======
      // This original line was removed (dead code)
>>>>>>> karim-radwan
      result.push({ original: o, optimized: null, status: 'removed' }); i++; continue
    }

    if (o === n) {
      result.push({ original: o, optimized: n, status: 'same' }); i++; j++; continue
    }

<<<<<<< HEAD
    // Check if it's a folded version of the same assignment (same LHS)
    const lhsO = o.match(/^([a-zA-Z_]\w*)\s*=/)?.[1]
    const lhsN = n.match(/^([a-zA-Z_]\w*)\s*=/)?.[1]
    if (lhsO && lhsN && lhsO === lhsN) {
      result.push({ original: o, optimized: n, status: 'folded' }); i++; j++; continue
    }

    // Line was removed
=======
    // Lines differ — determine why
    const tag = tagMap.get(o)

    if (tag === 'dead') {
      result.push({ original: o, optimized: null, status: 'removed' }); i++; continue
    }

    // Same LHS → the line was transformed (folded / propagated / cse)
    const lhsO = o.match(/^([a-zA-Z_]\w*)\s*=/)?.[1]
    const lhsN = n.match(/^([a-zA-Z_]\w*)\s*=/)?.[1]
    if (lhsO && lhsN && lhsO === lhsN) {
      const status: TACDiffLine['status'] =
        tag === 'cse'        ? 'cse'        :
        tag === 'propagated' ? 'propagated' :
        'folded'
      result.push({ original: o, optimized: n, status }); i++; j++; continue
    }

    // Fallback: treat as removed original
>>>>>>> karim-radwan
    result.push({ original: o, optimized: null, status: 'removed' }); i++
  }

  return result
}
