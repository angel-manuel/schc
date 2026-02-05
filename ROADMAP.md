# SCHC Compiler Roadmap: Haskell to C

## Current State

**Working Pipeline:**
```
Haskell Source → Lexer (Flex) → Parser → AST → coregen → Core IR → (nothing)
```

**Complete:** Lexer, Parser, AST (28 node types), Core IR (8 forms), Memory management, Environment/scoping

**Partial (coregen.c):** Functions, values, if-then-else, let, operators, variables, int literals

**Missing in coregen:** `AST_LAMBDA`, `AST_DO`, `AST_CASE`, `AST_DATA_DECL`, string literals

**Not started:** Type system (stub only), Code generation, Evaluation

---

## Design Decisions

| Decision | Choice | Rationale |
|----------|--------|-----------|
| Evaluation model | **Strict** | Lazy requires graph reduction, thunks, complex runtime |
| Type system | **Runtime tags first** | Works immediately; add static types later |
| Closures in C | **Tagged union + closure struct** | Handles partial application naturally |
| Memory | **malloc for now** | Add GC later if needed |

---

## Phase 1: Minimal Working Compiler

**Goal:** `main = fac 6` outputs `720`

### 1.1 Create Runtime Library

**New files:** `src/runtime/runtime.h`, `src/runtime/runtime.c`

```c
// Value representation
typedef struct schc_val {
    uint8_t tag;  // VAL_INT, VAL_CLOSURE, VAL_CON
    union {
        int64_t i64;
        struct { void *fn; int arity; int applied; schc_val_t **args; } closure;
    };
} schc_val_t;

// API
schc_val_t *schc_alloc_int(int64_t v);
schc_val_t *schc_alloc_closure(void *fn, int arity);
schc_val_t *schc_apply(schc_val_t *fn, schc_val_t *arg);

// Intrinsics
schc_val_t *schc_plus(schc_val_t *a, schc_val_t *b);
schc_val_t *schc_minus(schc_val_t *a, schc_val_t *b);
schc_val_t *schc_mult(schc_val_t *a, schc_val_t *b);
schc_val_t *schc_div(schc_val_t *a, schc_val_t *b);
schc_val_t *schc_lte(schc_val_t *a, schc_val_t *b);
schc_val_t *schc_gte(schc_val_t *a, schc_val_t *b);
schc_val_t *schc_eq(schc_val_t *a, schc_val_t *b);
```

### 1.2 Create Code Generator

**New files:** `src/codegen.h`, `src/codegen.c`

```c
typedef struct {
    FILE *output;
    int temp_counter;
} codegen_ctx_t;

int codegen_init(codegen_ctx_t *ctx, FILE *output);
int codegen_emit_program(codegen_ctx_t *ctx, const env_t *env);
```

**Translation patterns:**

| Core Form | C Output |
|-----------|----------|
| `42i64` | `schc_alloc_int(42)` |
| `APPL(#plus, a, b)` | `schc_plus(_ta, _tb)` |
| `LAMBDA {x} body` | Function + closure allocation |
| `COND` | `if (_cond->i64) { ... } else { ... }` |
| `INDIR(name)` | Reference to generated variable |

### 1.3 Update Main Driver

**Modify:** `src/schc.c`

- Add `-o output.c` flag
- After coregen, call `codegen_emit_program()`
- Generate complete C file with includes and main()

### 1.4 Update Build System

**Modify:** `Makefile`

- Compile runtime separately
- Add `compile` target: `.hs → .c → executable`
- Add `run` target for quick testing

### Phase 1 Tests

```haskell
-- test1.hs
main = 42

-- test2.hs
main = 2 + 3 * 4

-- test3.hs
double x = x + x
main = double 21

-- test4.hs (milestone)
fac n = if n <= 1 then 1 else n * fac (n - 1)
main = fac 6
```

---

## Phase 2: Strings + IO + Lambdas

**Goal:** `main = putStrLn "Hello, World!"`

### 2.1 String Literals

**Modify:** `src/core.h` - Add `CORE_LITERAL_STR`
**Modify:** `src/coregen.c` - Handle `AST_LIT_TYPE_STR`
**Modify:** `src/runtime/runtime.c` - Add `schc_alloc_str()`

### 2.2 IO Intrinsics

**Modify:** `src/runtime/runtime.c`

```c
void schc_putStrLn(schc_val_t *s);
schc_val_t *schc_show(schc_val_t *v);
```

For now, treat IO as direct side effects (no monad machinery).

### 2.3 Explicit Lambdas

**Modify:** `src/coregen.c` - Handle `AST_LAMBDA`

### Phase 2 Tests

```haskell
main = putStrLn "Hello, World!"
main = putStrLn (show (fac 6))
main = (\x -> x + 1) 41
```

---

## Phase 3: Data Types + Pattern Matching

**Goal:** User-defined types, case expressions

### 3.1 Data Declarations

**Modify:** `src/core.h` - Add data type representation
**Modify:** `src/coregen.c` - Handle `AST_DATA_DECL`

### 3.2 Case Expressions

**Modify:** `src/core.h` - Add `CORE_CASE`
**Modify:** `src/coregen.c` - Handle `AST_CASE`
**Modify:** `src/codegen.c` - Generate switch statements

### 3.3 Pattern Matching in Functions

Desugar multiple equations into single function with case.

### Phase 3 Tests

```haskell
data Maybe a = Nothing | Just a

fromMaybe def m = case m of
    Nothing -> def
    Just x -> x

main = fromMaybe 0 (Just 42)
```

---

## Phase 4: Do-Notation + Lists

**Goal:** Interactive programs, list operations

### 4.1 Do-Notation Desugaring

Transform `do { x <- e1; e2 }` into `e1 >>= (\x -> e2)`

### 4.2 List Support

Built-in list constructors and operations.

### 4.3 Optional: Monomorphic Type Checking

Add type checking pass before codegen for better error messages.

---

## File Summary

### New Files

| File | Phase | Purpose |
|------|-------|---------|
| `src/runtime/runtime.h` | 1 | Runtime API |
| `src/runtime/runtime.c` | 1 | Runtime implementation |
| `src/codegen.h` | 1 | Codegen API |
| `src/codegen.c` | 1 | C code generation |

### Modified Files

| File | Phase | Changes |
|------|-------|---------|
| `src/schc.c` | 1 | Add codegen invocation, -o flag |
| `src/core.h` | 2,3 | Add LITERAL_STR, CASE |
| `src/core.c` | 2,3 | Handle new forms |
| `src/coregen.c` | 2,3 | Handle LAMBDA, CASE, DATA_DECL |
| `Makefile` | 1 | Runtime build, compile target |

---

## Success Criteria

| Phase | Milestone |
|-------|-----------|
| 1 | `fac 10` compiles and outputs `3628800` |
| 2 | `putStrLn "Hello"` works |
| 3 | Maybe/List with pattern matching works |
| 4 | Interactive programs with do-notation |

---

## Implementation Order (Phase 1)

1. `src/runtime/runtime.h` - Define types and API
2. `src/runtime/runtime.c` - Implement runtime
3. `src/codegen.h` - Define codegen interface
4. `src/codegen.c` - Implement codegen for existing Core forms
5. `src/schc.c` - Integrate codegen into driver
6. `Makefile` - Add build targets
7. Test with `main = 42`, then `main = 2+2`, then `fac`
