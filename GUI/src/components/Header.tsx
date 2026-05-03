import { CompilationStatus } from '../App'
import './Header.css'

interface HeaderProps {
  status: CompilationStatus
  currentPhase: string
  compilationTime: number
  onCompile: () => void
  onReset: () => void
  onThemeToggle: () => void
  isDarkMode: boolean
}

function Header({ 
  status, 
  currentPhase, 
  compilationTime, 
  onCompile, 
  onReset,
  onThemeToggle,
  isDarkMode
}: HeaderProps) {
  const getStatusColor = () => {
    switch (status) {
      case 'idle': return 'var(--status-idle)'
      case 'compiling': return 'var(--status-running)'
      case 'success': return 'var(--status-success)'
      case 'error': return 'var(--status-error)'
      default: return 'var(--status-idle)'
    }
  }

  const getStatusText = () => {
    switch (status) {
      case 'idle': return 'Ready'
      case 'compiling': return 'Compiling...'
      case 'success': return 'Success'
      case 'error': return 'Error'
      default: return 'Ready'
    }
  }

  return (
    <header className="header">
      <div className="header-left">
        <div className="logo">
          <span className="logo-icon">🧠</span>
          <div className="logo-text-group">
            <span className="logo-text">Dr. Emad Compiler</span>
            <span className="logo-tagline">From Code to Execution — Step by Step</span>
          </div>
        </div>
        
        <div className="status-bar">
          <div className="status-item">
            <span 
              className="status-indicator" 
              style={{ backgroundColor: getStatusColor() }}
            />
            <span className="status-text">{getStatusText()}</span>
          </div>
          
          <div className="separator" />
          
          <div className="status-item">
            <span className="status-label">Phase:</span>
            <span className="status-value">{currentPhase}</span>
          </div>
          
          <div className="separator" />
          
          <div className="status-item">
            <span className="status-label">Time:</span>
            <span className="status-value">{compilationTime}ms</span>
          </div>
        </div>
      </div>
      
      <div className="header-right">
        <button 
          className="btn btn-primary" 
          onClick={onCompile}
          disabled={status === 'compiling'}
        >
          <span className="btn-icon">▶</span>
          <span className="btn-text">Compile</span>
        </button>
        
        <button 
          className="btn btn-ghost" 
          onClick={onReset}
          disabled={status === 'compiling'}
        >
          <span className="btn-icon">↻</span>
          <span className="btn-text">Reset</span>
        </button>
        
        <button 
          className="btn btn-icon" 
          onClick={onThemeToggle}
          title={isDarkMode ? "Switch to Light Mode" : "Switch to Dark Mode"}
        >
          <span>{isDarkMode ? 'Light' : 'Dark'}</span>
        </button>
      </div>
    </header>
  )
}

export default Header
