import './LearnView.css'

interface LearnViewProps {
  phases?: {
    lexical: boolean
    syntax: boolean
    semantic: boolean
    codegen: boolean
  }
}

function LearnView({ phases }: LearnViewProps) {
  const phaseData = [
    {
      id: 'lexical',
      name: 'Lexical Analysis',
      icon: '📝',
      description: 'Breaks source code into tokens (keywords, identifiers, operators, literals)',
      steps: [
        'Read source code character by character',
        'Identify keywords (int, char, string, return, etc.)',
        'Recognize identifiers (variable names)',
        'Extract literals (numbers, strings, characters)',
        'Detect operators (+, -, *, /, =, etc.)',
        'Generate token stream for parser'
      ],
      example: {
        input: 'int x = 42;',
        output: ['KEYWORD(int)', 'IDENTIFIER(x)', 'OPERATOR(=)', 'NUMBER(42)', 'SEMICOLON']
      }
    },
    {
      id: 'syntax',
      name: 'Syntax Analysis (Parsing)',
      icon: '🌳',
      description: 'Builds Abstract Syntax Tree (AST) using LR parsing algorithm',
      steps: [
        'Use LR parsing table (ACTION and GOTO)',
        'Process tokens using shift-reduce operations',
        'Build parse tree following grammar rules',
        'Detect syntax errors (missing semicolons, brackets)',
        'Generate Abstract Syntax Tree (AST)',
        'Validate program structure'
      ],
      example: {
        input: 'int x = 42;',
        output: ['DeclAssign', '├─ Type: int', '├─ Identifier: x', '└─ Literal: 42']
      }
    },
    {
      id: 'semantic',
      name: 'Semantic Analysis',
      icon: '🔍',
      description: 'Checks meaning and correctness of the program',
      steps: [
        'Build symbol table (track variables, types, scopes)',
        'Type checking (ensure type compatibility)',
        'Array bounds checking (prevent out-of-bounds access)',
        'Detect undeclared variables',
        'Check for duplicate declarations',
        'Validate array dimensions and sizes',
        'Annotate AST with type information'
      ],
      example: {
        input: 'int x[4]; int y = x[5];',
        output: ['❌ Error: Array index out of bounds', 'x[5] exceeds array size 4']
      }
    },
    {
      id: 'codegen',
      name: 'Code Generation',
      icon: '⚙️',
      description: 'Generates Three-Address Code (TAC) intermediate representation',
      steps: [
        'Traverse annotated AST',
        'Generate TAC instructions (DECL, ASSIGN, ADD, etc.)',
        'Handle array indexing (compute offsets)',
        'Generate temporaries for expressions',
        'Emit labels for control flow',
        'Produce optimizable intermediate code'
      ],
      example: {
        input: 'int z = x + y * 2;',
        output: ['t0 = y * 2', 't1 = x + t0', 'z = t1']
      }
    }
  ]

  return (
    <div className="learn-view">
      <div className="learn-header">
        <h2>🎓 Compiler Phases Explained</h2>
        <p>Learn how your code is compiled step-by-step</p>
      </div>

      <div className="phases-container">
        {phaseData.map((phase, index) => {
          const isPassed = phases ? phases[phase.id as keyof typeof phases] : false
          
          return (
            <div key={phase.id} className={`phase-card ${isPassed ? 'passed' : 'pending'}`}>
              <div className="phase-header">
                <div className="phase-number">{index + 1}</div>
                <div className="phase-icon">{phase.icon}</div>
                <div className="phase-title">
                  <h3>{phase.name}</h3>
                  <p>{phase.description}</p>
                </div>
                <div className="phase-status">
                  {isPassed ? (
                    <span className="status-badge success">✓ Passed</span>
                  ) : (
                    <span className="status-badge pending">⏳ Pending</span>
                  )}
                </div>
              </div>

              <div className="phase-body">
                <div className="phase-steps">
                  <h4>Steps:</h4>
                  <ol>
                    {phase.steps.map((step, i) => (
                      <li key={i}>{step}</li>
                    ))}
                  </ol>
                </div>

                <div className="phase-example">
                  <h4>Example:</h4>
                  <div className="example-box">
                    <div className="example-input">
                      <span className="example-label">Input:</span>
                      <code>{phase.example.input}</code>
                    </div>
                    <div className="example-arrow">→</div>
                    <div className="example-output">
                      <span className="example-label">Output:</span>
                      <div className="output-lines">
                        {phase.example.output.map((line, i) => (
                          <code key={i}>{line}</code>
                        ))}
                      </div>
                    </div>
                  </div>
                </div>
              </div>
            </div>
          )
        })}
      </div>

      <div className="learn-footer">
        <div className="info-box">
          <h3>💡 Key Concepts</h3>
          <ul>
            <li><strong>LR Parsing:</strong> Bottom-up parsing using shift-reduce operations</li>
            <li><strong>Symbol Table:</strong> Data structure storing variable information</li>
            <li><strong>TAC:</strong> Three-Address Code - intermediate representation</li>
            <li><strong>AST:</strong> Abstract Syntax Tree - hierarchical program structure</li>
          </ul>
        </div>

        <div className="info-box">
          <h3>🎯 Learning Tips</h3>
          <ul>
            <li>Compile simple programs first to understand each phase</li>
            <li>Use Step Mode to see TAC execution line-by-line</li>
            <li>Check the Symbol Table to see variable metadata</li>
            <li>Examine the AST to understand program structure</li>
          </ul>
        </div>
      </div>
    </div>
  )
}

export default LearnView
