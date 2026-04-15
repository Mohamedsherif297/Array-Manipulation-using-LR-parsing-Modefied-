### CFG

0: S → Program
1: Program → StmtList

2: StmtList → StmtList Stmt
3: StmtList → Stmt

4: Stmt → DeclStmt
5: Stmt → AssignStmt
6: Stmt → DeclAssignStmt

7: DeclStmt → Type ID ;
8: DeclStmt → Type ID ArrayDims ;

9: AssignStmt → ID = Expr ;
10: AssignStmt → ArrayAccess = Expr ;

11: DeclAssignStmt → Type ID = Expr ;
12: DeclAssignStmt → Type ID ArrayDims = ArrayLiteral ;

13: ArrayLiteral → { RowList }
14: RowList → RowList , Row
15: RowList → Row

16: Row → { ExprList }
17: Row → Expr

18: ExprList → ExprList , Expr
19: ExprList → Expr

20: ArrayAccess → ID [ Expr ]

21: ArrayDims → ArrayDims [ NUM ]
22: ArrayDims → [ NUM ]

23: Expr → Expr + Term
24: Expr → Expr - Term
25: Expr → Term

26: Term → Term * Factor
27: Term → Term / Factor
28: Term → Factor

29: Factor → ( Expr )
30: Factor → ID
31: Factor → NUM
32: Factor → ArrayAccess
33: Factor → STRING
34: Factor → CHAR

35: Type → int
36: Type → float
37: Type → double
38: Type → char
39: Type → string