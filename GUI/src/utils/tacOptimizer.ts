// tacOptimizer.ts
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

function parseLine(raw: string): TACLine {
  const annotation = raw.match(/(\s*;\s*line:\d+\s*)$/)?.[1] ?? ''
  const clean = raw.replace(/\s*;\s*line:\d+\s*$/, '').trim()

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
    return base
  }

  // result = arg1[arg2]  (LOAD)
  const loadM = clean.match(/^([a-zA-Z_]\w*)\s*=\s*([a-zA-Z_]\w*)\[(.+?)\]$/)
  if (loadM) {
    base.result = loadM[1]
    base.arg1 = loadM[2]
    base.arg2 = loadM[3]
    base.op = 'LOAD'
    return base
  }

  // result = arg1  (ASSIGN)
  const assignM = clean.match(/^([a-zA-Z_]\w*)\s*=\s*(.+)$/)
  if (assignM) {
    base.result = assignM[1]
    base.arg1 = assignM[2].trim()
    return base
  }

  return base
}

function isNumeric(s: string): boolean {
  return /^-?\d+(\.\d+)?$/.test(s.trim())
}

function foldBinary(a: string, op: string, b: string): string | null {
  if (!isNumeric(a) || !isNumeric(b)) return null
  const na = parseFloat(a), nb = parseFloat(b)
  let result: number
  if (op === '+') result = na + nb
  else if (op === '-') result = na - nb
  else if (op === '*') result = na * nb
  else if (op === '/') { if (nb === 0) return null; result = na / nb }
  else return null
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
        }
      }
    }

    return line
  })

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

  const result: TACDiffLine[] = []
  let i = 0, j = 0

  while (i < origLines.length || j < optLines.length) {
    const o = origLines[i]
    const n = optLines[j]

    if (i >= origLines.length) {
      result.push({ original: null, optimized: n, status: 'added' }); j++; continue
    }
    if (j >= optLines.length) {
      result.push({ original: o, optimized: null, status: 'removed' }); i++; continue
    }

    if (o === n) {
      result.push({ original: o, optimized: n, status: 'same' }); i++; j++; continue
    }

    // Check if it's a folded version of the same assignment (same LHS)
    const lhsO = o.match(/^([a-zA-Z_]\w*)\s*=/)?.[1]
    const lhsN = n.match(/^([a-zA-Z_]\w*)\s*=/)?.[1]
    if (lhsO && lhsN && lhsO === lhsN) {
      result.push({ original: o, optimized: n, status: 'folded' }); i++; j++; continue
    }

    // Line was removed
    result.push({ original: o, optimized: null, status: 'removed' }); i++
  }

  return result
}
