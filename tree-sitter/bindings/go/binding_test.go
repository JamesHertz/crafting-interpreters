package tree_sitter_lox_test

import (
	"testing"

	tree_sitter "github.com/tree-sitter/go-tree-sitter"
	tree_sitter_lox "github.com/tree-sitter/tree-sitter-lox/bindings/go"
)

func TestCanLoadGrammar(t *testing.T) {
	language := tree_sitter.NewLanguage(tree_sitter_lox.Language())
	if language == nil {
		t.Errorf("Error loading Lox parser grammar")
	}
}
