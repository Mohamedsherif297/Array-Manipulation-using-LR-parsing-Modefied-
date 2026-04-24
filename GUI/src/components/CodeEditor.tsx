import { useEffect, useRef } from 'react'
import Editor from '@monaco-editor/react'
import type { editor } from 'monaco-editor'
import { CompilerError } from '../App'
import './CodeEditor.css'

interface CodeEditorProps {
  value: string
  onChange: (value: string) => void
  highlightedLine?: number | null
  errors?: CompilerError[]
  onLineClick?: (lineNumber: number) => void
}

function CodeEditor({ value, onChange, highlightedLine, errors = [], onLineClick }: CodeEditorProps) {
  const editorRef = useRef<editor.IStandaloneCodeEditor | null>(null)
  const decorationsRef = useRef<string[]>([])

  const handleEditorChange = (value: string | undefined) => {
    onChange(value || '')
  }

  const handleEditorDidMount = (editor: editor.IStandaloneCodeEditor) => {
    editorRef.current = editor

    // Add click handler for lines
    editor.onMouseDown((e) => {
      if (e.target.position && onLineClick) {
        onLineClick(e.target.position.lineNumber)
      }
    })
  }

  // Update decorations when errors or highlighted line changes
  useEffect(() => {
    if (!editorRef.current) return

    const editor = editorRef.current
    const monaco = (window as any).monaco

    if (!monaco) return

    const newDecorations: editor.IModelDeltaDecoration[] = []

    // Add error decorations
    errors.forEach((error) => {
      if (error.line) {
        newDecorations.push({
          range: new monaco.Range(error.line, 1, error.line, 1),
          options: {
            isWholeLine: true,
            className: 'error-line',
            glyphMarginClassName: 'error-glyph',
            hoverMessage: { value: `**${error.phase.toUpperCase()} ERROR**: ${error.message}` },
            minimap: {
              color: '#f48771',
              position: monaco.editor.MinimapPosition.Inline
            }
          }
        })

        // Add squiggly underline
        newDecorations.push({
          range: new monaco.Range(error.line, 1, error.line, 1000),
          options: {
            inlineClassName: 'error-squiggly'
          }
        })
      }
    })

    // Add highlighted line decoration
    if (highlightedLine) {
      newDecorations.push({
        range: new monaco.Range(highlightedLine, 1, highlightedLine, 1),
        options: {
          isWholeLine: true,
          className: 'highlighted-line',
          glyphMarginClassName: 'highlighted-glyph'
        }
      })
    }

    // Apply decorations
    decorationsRef.current = editor.deltaDecorations(decorationsRef.current, newDecorations)
  }, [errors, highlightedLine])

  return (
    <div className="code-editor">
      <Editor
        height="100%"
        defaultLanguage="cpp"
        theme="vs-dark"
        value={value}
        onChange={handleEditorChange}
        onMount={handleEditorDidMount}
        options={{
          minimap: { enabled: true },
          fontSize: 14,
          lineNumbers: 'on',
          roundedSelection: true,
          scrollBeyondLastLine: false,
          automaticLayout: true,
          tabSize: 2,
          wordWrap: 'on',
          padding: { top: 16, bottom: 16 },
          glyphMargin: true,
          folding: true,
          lineDecorationsWidth: 10,
          lineNumbersMinChars: 3,
        }}
      />
    </div>
  )
}

export default CodeEditor
