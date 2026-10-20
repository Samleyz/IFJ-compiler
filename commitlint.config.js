export default {
    extends: ['@commitlint/config-conventional'],
    rules: {
        // 1. Type (feat, fix, chore...) is strictly required
        // (2 = error, 'never' = must not be empty)
        'type-empty': [2, 'never'],

        // 2. Scope (module) is optional, but triggers a warning if missing
        // (1 = warning, 'never' = should not be empty)
        'scope-empty': [1, 'never'],

        // 3. If a scope is provided, it MUST be one of the following
        // (2 = error on typo or unknown module)
        'scope-enum': [
          2,
          'always',
          [
            'scanner',   // Lexical analysis
            'parser',    // Syntax analysis
            'symtable',  // Symbol table / semantics
            'adt',       // Abstract data types
            'codegen',   // Target code generation
            'expr',      // Expression parsing (Precedence analysis)
            'docs',      // Documentation (README, etc.)
            'setup',     // Repository configuration
            'test',      // Test scripts and test cases
			'utils',     // Shared utilities
			'core'       // Main orchestration and entry point
            ]
        ]
    }
};
