import { useState, useRef, useEffect } from 'react'
import { CompilerError } from '../App'
import './EditorPanel.css'

interface EditorPanelProps {
  code: string
  onChange: (code: string) => void
  highlightedLine: number | null
  errors?: CompilerError[]
  onLineClick: (line: number) => void
  canUndo: boolean
  canRedo: boolean
  onUndo: () => void
  onRedo: () => void
}

function EditorPanel({ code, onChange, highlightedLine, errors, onLineClick, canUndo, canRedo, onUndo, onRedo }: EditorPanelProps) {
  const [cursorPosition, setCursorPosition] = useState({ line: 1, col: 1 })
  const textareaRef = useRef<HTMLTextAreaElement>(null)
  const lineNumbersRef = useRef<HTMLDivElement>(null)
  const codeAreaRef = useRef<HTMLDivElement>(null)
  
  const lines = code.split('\n')
  const lineCount = lines.length
  const charCount = code.length

  // Get errors by line number
  const errorsByLine = new Map<number, CompilerError[]>()
  errors?.forEach(error => {
    if (error.line) {
      if (!errorsByLine.has(error.line)) {
        errorsByLine.set(error.line, [])
      }
      errorsByLine.get(error.line)!.push(error)
    }
  })

  // Handle key down for auto-completion and tab indentation
  const handleKeyDown = (e: React.KeyboardEvent<HTMLTextAreaElement>) => {
    if ((e.ctrlKey || e.metaKey) && e.key.toLowerCase() === 'z') {
      e.preventDefault()
      if (e.shiftKey) {
        onRedo()
      } else {
        onUndo()
      }
      return
    }

    if ((e.ctrlKey || e.metaKey) && e.key.toLowerCase() === 'y') {
      e.preventDefault()
      onRedo()
      return
    }

    const textarea = e.currentTarget
    const start = textarea.selectionStart
    const end = textarea.selectionEnd
    const value = textarea.value

    // Handle TAB key for indentation
    if (e.key === 'Tab') {
      e.preventDefault()
      
      const beforeCursor = value.substring(0, start)
      const afterCursor = value.substring(end)
      
      // Insert 4 spaces for indentation
      const newValue = beforeCursor + '    ' + afterCursor
      onChange(newValue)
      
      // Set cursor position after the inserted spaces
      setTimeout(() => {
        textarea.selectionStart = textarea.selectionEnd = start + 4
      }, 0)
      return
    }

    // Auto-complete brackets and braces
    const pairs: { [key: string]: string } = {
      '(': ')',
      '[': ']',
      '{': '}',
      '"': '"',
      "'": "'"
    }

    if (pairs[e.key]) {
      e.preventDefault()
      
      const beforeCursor = value.substring(0, start)
      const afterCursor = value.substring(end)
      const selectedText = value.substring(start, end)
      
      // If text is selected, wrap it
      if (start !== end) {
        const newValue = beforeCursor + e.key + selectedText + pairs[e.key] + afterCursor
        onChange(newValue)
        setTimeout(() => {
          textarea.selectionStart = start + 1
          textarea.selectionEnd = end + 1
        }, 0)
      } else {
        // Insert pair and place cursor between them
        const newValue = beforeCursor + e.key + pairs[e.key] + afterCursor
        onChange(newValue)
        setTimeout(() => {
          textarea.selectionStart = textarea.selectionEnd = start + 1
        }, 0)
      }
      return
    }

    // Auto-indent on Enter
    if (e.key === 'Enter') {
      e.preventDefault()
      
      const beforeCursor = value.substring(0, start)
      const afterCursor = value.substring(end)
      
      // Get current line
      const currentLineStart = beforeCursor.lastIndexOf('\n') + 1
      const currentLine = beforeCursor.substring(currentLineStart)
      
      // Calculate indentation of current line
      const indentMatch = currentLine.match(/^(\s*)/)
      const currentIndent = indentMatch ? indentMatch[1] : ''
      
      // Check if we just opened a brace
      const lastChar = beforeCursor.trim().slice(-1)
      const nextChar = afterCursor.trim()[0]
      
      let newValue: string
      let cursorOffset: number
      
      if (lastChar === '{' && nextChar === '}') {
        // Between braces - add extra indentation and closing brace line
        newValue = beforeCursor + '\n' + currentIndent + '    \n' + currentIndent + afterCursor
        cursorOffset = start + 1 + currentIndent.length + 4
      } else if (lastChar === '{') {
        // After opening brace - add indentation
        newValue = beforeCursor + '\n' + currentIndent + '    ' + afterCursor
        cursorOffset = start + 1 + currentIndent.length + 4
      } else {
        // Normal enter - maintain indentation
        newValue = beforeCursor + '\n' + currentIndent + afterCursor
        cursorOffset = start + 1 + currentIndent.length
      }
      
      onChange(newValue)
      setTimeout(() => {
        textarea.selectionStart = textarea.selectionEnd = cursorOffset
      }, 0)
      return
    }

    // Auto-close when typing closing bracket after auto-completed pair
    if (e.key === ')' || e.key === ']' || e.key === '}') {
      const nextChar = value[start]
      if (nextChar === e.key) {
        e.preventDefault()
        // Just move cursor forward
        setTimeout(() => {
          textarea.selectionStart = textarea.selectionEnd = start + 1
        }, 0)
        return
      }
    }
  }

  // Update cursor position
  const updateCursorPosition = () => {
    if (textareaRef.current) {
      const textarea = textareaRef.current
      const text = textarea.value.substring(0, textarea.selectionStart)
      const lines = text.split('\n')
      const line = lines.length
      const col = lines[lines.length - 1].length + 1
      setCursorPosition({ line, col })
    }
  }

  useEffect(() => {
    updateCursorPosition()
  }, [code])

  // Sync line numbers scroll with code area scroll
  useEffect(() => {
    const codeArea = codeAreaRef.current
    const lineNumbers = lineNumbersRef.current
    if (!codeArea || !lineNumbers) return

    const handleScroll = () => {
      lineNumbers.scrollTop = codeArea.scrollTop
    }

    codeArea.addEventListener('scroll', handleScroll)
    return () => codeArea.removeEventListener('scroll', handleScroll)
  }, [])

  // Resize textarea to match content so .code-area scrolls the container
  useEffect(() => {
    const textarea = textareaRef.current
    if (!textarea) return
    textarea.style.height = 'auto'
    textarea.style.height = textarea.scrollHeight + 'px'
    textarea.style.width = Math.max(textarea.scrollWidth, textarea.clientWidth) + 'px'
  }, [code])

  // Scroll to highlighted line when it changes
  useEffect(() => {
    if (highlightedLine !== null && codeAreaRef.current) {
      const lineHeight = 22.4 // Must match the line height in CSS
      const targetScrollTop = (highlightedLine - 1) * lineHeight
      const containerHeight = codeAreaRef.current.clientHeight
      const currentScrollTop = codeAreaRef.current.scrollTop
      
      // Only scroll if the line is not visible in the current viewport
      const lineTop = targetScrollTop
      const lineBottom = targetScrollTop + lineHeight
      const viewportTop = currentScrollTop
      const viewportBottom = currentScrollTop + containerHeight
      
      if (lineTop < viewportTop || lineBottom > viewportBottom) {
        // Center the line in the viewport
        codeAreaRef.current.scrollTo({
          top: targetScrollTop - containerHeight / 2 + lineHeight / 2,
          behavior: 'smooth'
        })
      }
    }
  }, [highlightedLine])

  // Syntax highlighting - process in correct order to avoid conflicts
  const highlightSyntax = (line: string) => {
    if (!line) return '&nbsp;'
    
    const tokens: Array<{ text: string; type: string }> = []
    let remaining = line
    
    // Regex patterns
    const patterns = [
      { type: 'comment', regex: /^(\/\/.*)/ },
      { type: 'string', regex: /^("(?:[^"\\]|\\.)*")/ },
      { type: 'char', regex: /^('(?:[^'\\]|\\.)')/ },
      { type: 'keyword', regex: /^(return|if|else|while|for|cout|cin)\b/ },
      { type: 'type', regex: /^(int|char|string|void)\b/ },
      { type: 'function', regex: /^(main)\b(?=\s*\()/ },
      { type: 'number', regex: /^(\d+)/ },
      { type: 'operator', regex: /^([+\-*/%=<>!&|]+|<<|>>)/ },
      { type: 'punctuation', regex: /^([{}()\[\];,])/ },
      { type: 'identifier', regex: /^([a-zA-Z_]\w*)/ },
      { type: 'whitespace', regex: /^(\s+)/ },
    ]
    
    while (remaining.length > 0) {
      let matched = false
      
      for (const pattern of patterns) {
        const match = remaining.match(pattern.regex)
        if (match) {
          tokens.push({ text: match[1], type: pattern.type })
          remaining = remaining.slice(match[1].length)
          matched = true
          break
        }
      }
      
      if (!matched) {
        tokens.push({ text: remaining[0], type: 'text' })
        remaining = remaining.slice(1)
      }
    }
    
    // Build HTML
    return tokens.map(token => {
      if (token.type === 'whitespace') return token.text
      if (token.type === 'text') return token.text
      return `<span class="syntax-${token.type}">${token.text}</span>`
    }).join('')
  }

  return (
    <div className="editor-panel">
      <div className="editor-header">
        <div className="editor-tabs">
          <div className="editor-tab active">
            <span className="tab-icon">C</span>
            <span className="tab-label">main.c</span>
          </div>
        </div>
        <div className="editor-actions">
          <button
            className="icon-btn"
            onClick={onUndo}
            disabled={!canUndo}
            title="Undo (Ctrl+Z)"
          >
            ↶
          </button>
          <button
            className="icon-btn"
            onClick={onRedo}
            disabled={!canRedo}
            title="Redo (Ctrl+Y)"
          >
            ↷
          </button>
        </div>
      </div>

      <div className="editor-content">
        <div className="line-numbers" ref={lineNumbersRef}>
          {lines.map((_, index) => {
            const lineNum = index + 1
            const hasError = errorsByLine.has(lineNum)
            const isHighlighted = lineNum === highlightedLine
            
            return (
              <div
                key={lineNum}
                className={`line-number ${hasError ? 'has-error' : ''} ${isHighlighted ? 'highlighted' : ''}`}
                onClick={() => onLineClick(lineNum)}
                title={hasError ? errorsByLine.get(lineNum)![0].message : ''}
              >
                {lineNum}
              </div>
            )
          })}
        </div>

        <div className="code-area" ref={codeAreaRef}>
          <textarea
            ref={textareaRef}
            className="code-input"
            value={code}
            onChange={(e) => onChange(e.target.value)}
            onKeyUp={updateCursorPosition}
            onKeyDown={handleKeyDown}
            onClick={updateCursorPosition}
            spellCheck={false}
            autoCapitalize="off"
            autoComplete="off"
            autoCorrect="off"
          />
          
          <div className="code-highlight" aria-hidden="true">
            {lines.map((line, index) => {
              const lineNum = index + 1
              const hasError = errorsByLine.has(lineNum)
              return (
                <div
                  key={index}
                  className={`code-line ${lineNum === highlightedLine ? 'highlighted' : ''} ${hasError ? 'error-line-bg' : ''}`}
                  dangerouslySetInnerHTML={{ __html: highlightSyntax(line) || '&nbsp;' }}
                />
              )
            })}
          </div>

          {/* Inline error highlights with tooltips */}
          {Array.from(errorsByLine.entries()).map(([lineNum, lineErrors]) => (
            <div
              key={lineNum}
              className="error-line-highlight"
              style={{ top: `${(lineNum - 1) * 22.4 + 12}px` }}
            >
              <div className="error-squiggle-line" />
              <div className="error-inline-tooltip">
                {lineErrors.map((e, i) => (
                  <div key={i} className="error-tooltip-item">
                    <span className={`error-tooltip-phase error-phase-${e.phase}`}>
                      {e.phase.toUpperCase()}
                    </span>
                    <span className="error-tooltip-msg">{e.message}</span>
                  </div>
                ))}
              </div>
            </div>
          ))}
        </div>
      </div>

      <div className="editor-footer">
        <div className="footer-item">
          <span>Ln {cursorPosition.line}, Col {cursorPosition.col}</span>
        </div>
        <div className="footer-item">
          <span>UTF-8</span>
        </div>
        <div className="footer-item">
          <span>C Language</span>
        </div>
        <div className="footer-item">
          <span>{charCount} chars</span>
        </div>
        <div className="footer-item">
          <span>{lineCount} lines</span>
        </div>
      </div>
    </div>
  )
}

export default EditorPanel
