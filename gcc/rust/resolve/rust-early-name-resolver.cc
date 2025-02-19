// Copyright (C) 2020-2022 Free Software Foundation, Inc.

// This file is part of GCC.

// GCC is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3, or (at your option) any later
// version.

// GCC is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.

// You should have received a copy of the GNU General Public License
// along with GCC; see the file COPYING3.  If not see
// <http://www.gnu.org/licenses/>.

#include "rust-early-name-resolver.h"
#include "rust-ast-full.h"
#include "rust-name-resolver.h"

namespace Rust {
namespace Resolver {

EarlyNameResolver::EarlyNameResolver ()
  : resolver (*Resolver::get ()), mappings (*Analysis::Mappings::get ())
{}

void
EarlyNameResolver::go (AST::Crate &crate)
{
  // FIXME: Is that valid? Why is the regular name resolution doing
  // mappings->get_next_node_id()?
  resolver.get_macro_scope ().push (crate.get_node_id ());

  for (auto &item : crate.items)
    item->accept_vis (*this);

  // FIXME: Should we pop the macro scope?
}

void
EarlyNameResolver::resolve_generic_args (AST::GenericArgs &generic_args)
{
  for (auto &arg : generic_args.get_generic_args ())
    arg.accept_vis (*this);

  for (auto &arg : generic_args.get_binding_args ())
    arg.get_type ()->accept_vis (*this);
}

void
EarlyNameResolver::resolve_qualified_path_type (AST::QualifiedPathType &path)
{
  path.get_type ()->accept_vis (*this);

  if (path.has_as_clause ())
    path.get_as_type_path ().accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::Token &tok)
{}

void
EarlyNameResolver::visit (AST::DelimTokenTree &delim_tok_tree)
{}

void
EarlyNameResolver::visit (AST::AttrInputMetaItemContainer &input)
{}

void
EarlyNameResolver::visit (AST::IdentifierExpr &ident_expr)
{}

void
EarlyNameResolver::visit (AST::Lifetime &lifetime)
{}

void
EarlyNameResolver::visit (AST::LifetimeParam &lifetime_param)
{}

void
EarlyNameResolver::visit (AST::ConstGenericParam &const_param)
{}

// FIXME: ARTHUR: Do we need to perform macro resolution for paths as well?
// std::arch::asm!()?
void
EarlyNameResolver::visit (AST::PathInExpression &path)
{
  for (auto &segment : path.get_segments ())
    if (segment.has_generic_args ())
      resolve_generic_args (segment.get_generic_args ());
}

void
EarlyNameResolver::visit (AST::TypePathSegment &segment)
{}

void
EarlyNameResolver::visit (AST::TypePathSegmentGeneric &segment)
{
  if (segment.has_generic_args ())
    resolve_generic_args (segment.get_generic_args ());
}

void
EarlyNameResolver::visit (AST::TypePathSegmentFunction &segment)
{
  for (auto &type : segment.get_type_path_function ().get_params ())
    type->accept_vis (*this);

  segment.get_type_path_function ().get_return_type ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::TypePath &path)
{
  for (auto &seg : path.get_segments ())
    seg->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::QualifiedPathInExpression &path)
{
  resolve_qualified_path_type (path.get_qualified_path_type ());

  for (auto &segment : path.get_segments ())
    if (segment.has_generic_args ())
      resolve_generic_args (segment.get_generic_args ());
}

void
EarlyNameResolver::visit (AST::QualifiedPathInType &path)
{
  resolve_qualified_path_type (path.get_qualified_path_type ());

  for (auto &segment : path.get_segments ())
    segment->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::LiteralExpr &expr)
{}

void
EarlyNameResolver::visit (AST::AttrInputLiteral &attr_input)
{}

void
EarlyNameResolver::visit (AST::MetaItemLitExpr &meta_item)
{}

void
EarlyNameResolver::visit (AST::MetaItemPathLit &meta_item)
{}

void
EarlyNameResolver::visit (AST::BorrowExpr &expr)
{
  expr.get_borrowed_expr ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::DereferenceExpr &expr)
{
  expr.get_dereferenced_expr ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::ErrorPropagationExpr &expr)
{
  expr.get_propagating_expr ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::NegationExpr &expr)
{
  expr.get_negated_expr ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::ArithmeticOrLogicalExpr &expr)
{
  expr.get_left_expr ()->accept_vis (*this);
  expr.get_right_expr ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::ComparisonExpr &expr)
{
  expr.get_left_expr ()->accept_vis (*this);
  expr.get_right_expr ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::LazyBooleanExpr &expr)
{
  expr.get_left_expr ()->accept_vis (*this);
  expr.get_right_expr ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::TypeCastExpr &expr)
{
  expr.get_casted_expr ()->accept_vis (*this);
  expr.get_type_to_cast_to ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::AssignmentExpr &expr)
{
  expr.get_left_expr ()->accept_vis (*this);
  expr.get_right_expr ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::CompoundAssignmentExpr &expr)
{
  expr.get_left_expr ()->accept_vis (*this);
  expr.get_right_expr ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::GroupedExpr &expr)
{
  expr.get_expr_in_parens ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::ArrayElemsValues &elems)
{
  for (auto &expr : elems.get_values ())
    expr->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::ArrayElemsCopied &elems)
{
  elems.get_elem_to_copy ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::ArrayExpr &expr)
{
  expr.get_array_elems ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::ArrayIndexExpr &expr)
{
  expr.get_array_expr ()->accept_vis (*this);
  expr.get_index_expr ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::TupleExpr &expr)
{
  for (auto &elem : expr.get_tuple_elems ())
    elem->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::TupleIndexExpr &expr)
{
  expr.get_tuple_expr ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::StructExprStruct &expr)
{}

void
EarlyNameResolver::visit (AST::StructExprFieldIdentifier &field)
{}

void
EarlyNameResolver::visit (AST::StructExprFieldIdentifierValue &field)
{
  field.get_value ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::StructExprFieldIndexValue &field)
{
  field.get_value ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::StructExprStructFields &expr)
{
  for (auto &field : expr.get_fields ())
    field->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::StructExprStructBase &expr)
{}

void
EarlyNameResolver::visit (AST::CallExpr &expr)
{
  expr.get_function_expr ()->accept_vis (*this);
  for (auto &param : expr.get_params ())
    param->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::MethodCallExpr &expr)
{
  expr.get_receiver_expr ()->accept_vis (*this);
  for (auto &param : expr.get_params ())
    param->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::FieldAccessExpr &expr)
{
  expr.get_receiver_expr ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::ClosureExprInner &expr)
{
  expr.get_definition_expr ()->accept_vis (*this);

  for (auto &param : expr.get_params ())
    param.get_type ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::BlockExpr &expr)
{
  for (auto &stmt : expr.get_statements ())
    stmt->accept_vis (*this);

  if (expr.has_tail_expr ())
    expr.get_tail_expr ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::ClosureExprInnerTyped &expr)
{
  expr.get_definition_block ()->accept_vis (*this);

  for (auto &param : expr.get_params ())
    param.get_type ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::ContinueExpr &expr)
{}

void
EarlyNameResolver::visit (AST::BreakExpr &expr)
{
  if (expr.has_break_expr ())
    expr.get_break_expr ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::RangeFromToExpr &expr)
{
  expr.get_from_expr ()->accept_vis (*this);
  expr.get_to_expr ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::RangeFromExpr &expr)
{
  expr.get_from_expr ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::RangeToExpr &expr)
{
  expr.get_to_expr ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::RangeFullExpr &expr)
{}

void
EarlyNameResolver::visit (AST::RangeFromToInclExpr &expr)
{
  expr.get_from_expr ()->accept_vis (*this);
  expr.get_to_expr ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::RangeToInclExpr &expr)
{
  expr.get_to_expr ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::ReturnExpr &expr)
{
  if (expr.has_returned_expr ())
    expr.get_returned_expr ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::UnsafeBlockExpr &expr)
{
  expr.get_block_expr ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::LoopExpr &expr)
{
  expr.get_loop_block ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::WhileLoopExpr &expr)
{
  expr.get_predicate_expr ()->accept_vis (*this);
  expr.get_loop_block ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::WhileLetLoopExpr &expr)
{
  expr.get_scrutinee_expr ()->accept_vis (*this);
  expr.get_loop_block ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::ForLoopExpr &expr)
{
  expr.get_iterator_expr ()->accept_vis (*this);
  expr.get_loop_block ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::IfExpr &expr)
{
  expr.get_condition_expr ()->accept_vis (*this);
  expr.get_if_block ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::IfExprConseqElse &expr)
{
  expr.get_condition_expr ()->accept_vis (*this);
  expr.get_if_block ()->accept_vis (*this);
  expr.get_else_block ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::IfExprConseqIf &expr)
{
  expr.get_condition_expr ()->accept_vis (*this);
  expr.get_if_block ()->accept_vis (*this);
  expr.get_conseq_if_expr ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::IfExprConseqIfLet &expr)
{
  expr.get_condition_expr ()->accept_vis (*this);
  expr.get_if_block ()->accept_vis (*this);
  expr.get_conseq_if_let_expr ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::IfLetExpr &expr)
{
  expr.get_value_expr ()->accept_vis (*this);
  expr.get_if_block ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::IfLetExprConseqElse &expr)
{
  expr.get_value_expr ()->accept_vis (*this);
  expr.get_if_block ()->accept_vis (*this);
  expr.get_else_block ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::IfLetExprConseqIf &expr)
{
  expr.get_value_expr ()->accept_vis (*this);
  expr.get_if_block ()->accept_vis (*this);
  expr.get_conseq_if_expr ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::IfLetExprConseqIfLet &expr)
{
  expr.get_value_expr ()->accept_vis (*this);
  expr.get_if_block ()->accept_vis (*this);
  expr.get_conseq_if_let_expr ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::MatchExpr &expr)
{
  expr.get_scrutinee_expr ()->accept_vis (*this);
  for (auto &match_arm : expr.get_match_cases ())
    {
      if (match_arm.get_arm ().has_match_arm_guard ())
	match_arm.get_arm ().get_guard_expr ()->accept_vis (*this);

      for (auto &pattern : match_arm.get_arm ().get_patterns ())
	pattern->accept_vis (*this);

      match_arm.get_expr ()->accept_vis (*this);
    }
}

void
EarlyNameResolver::visit (AST::AwaitExpr &expr)
{
  expr.get_awaited_expr ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::AsyncBlockExpr &expr)
{
  expr.get_block_expr ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::TypeParam &param)
{
  for (auto &bound : param.get_type_param_bounds ())
    bound->accept_vis (*this);

  if (param.has_type ())
    param.get_type ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::LifetimeWhereClauseItem &item)
{}

void
EarlyNameResolver::visit (AST::TypeBoundWhereClauseItem &item)
{
  for (auto &bound : item.get_type_param_bounds ())
    bound->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::Method &method)
{
  if (method.has_generics ())
    for (auto &generic : method.get_generic_params ())
      generic->accept_vis (*this);

  if (method.get_self_param ().has_type ())
    method.get_self_param ().get_type ()->accept_vis (*this);

  for (auto &param : method.get_function_params ())
    param.get_type ()->accept_vis (*this);

  if (method.has_return_type ())
    method.get_return_type ()->accept_vis (*this);

  method.get_definition ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::Module &module)
{
  for (auto &item : module.get_items ())
    item->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::ExternCrate &crate)
{}

void
EarlyNameResolver::visit (AST::UseTreeGlob &use_tree)
{}

void
EarlyNameResolver::visit (AST::UseTreeList &use_tree)
{}

void
EarlyNameResolver::visit (AST::UseTreeRebind &use_tree)
{}

void
EarlyNameResolver::visit (AST::UseDeclaration &use_decl)
{}

void
EarlyNameResolver::visit (AST::Function &function)
{
  if (function.has_generics ())
    for (auto &generic : function.get_generic_params ())
      generic->accept_vis (*this);

  for (auto &param : function.get_function_params ())
    param.get_type ()->accept_vis (*this);

  if (function.has_return_type ())
    function.get_return_type ()->accept_vis (*this);

  function.get_definition ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::TypeAlias &type_alias)
{
  type_alias.get_type_aliased ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::StructStruct &struct_item)
{
  for (auto &field : struct_item.get_fields ())
    field.get_field_type ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::TupleStruct &tuple_struct)
{
  for (auto &field : tuple_struct.get_fields ())
    field.get_field_type ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::EnumItem &item)
{}

void
EarlyNameResolver::visit (AST::EnumItemTuple &item)
{}

void
EarlyNameResolver::visit (AST::EnumItemStruct &item)
{}

void
EarlyNameResolver::visit (AST::EnumItemDiscriminant &item)
{}

void
EarlyNameResolver::visit (AST::Enum &enum_item)
{}

void
EarlyNameResolver::visit (AST::Union &union_item)
{}

void
EarlyNameResolver::visit (AST::ConstantItem &const_item)
{
  const_item.get_type ()->accept_vis (*this);
  const_item.get_expr ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::StaticItem &static_item)
{
  static_item.get_type ()->accept_vis (*this);
  static_item.get_expr ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::TraitItemFunc &item)
{
  auto &decl = item.get_trait_function_decl ();

  if (decl.has_return_type ())
    decl.get_return_type ()->accept_vis (*this);

  for (auto &generic : decl.get_generic_params ())
    generic->accept_vis (*this);

  for (auto &param : decl.get_function_params ())
    param.get_type ()->accept_vis (*this);

  if (item.has_definition ())
    item.get_definition ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::TraitItemMethod &item)
{
  // FIXME: Can we factor this with the above function?
  auto &decl = item.get_trait_method_decl ();

  if (decl.has_return_type ())
    decl.get_return_type ()->accept_vis (*this);

  for (auto &generic : decl.get_generic_params ())
    generic->accept_vis (*this);

  for (auto &param : decl.get_function_params ())
    param.get_type ()->accept_vis (*this);

  if (item.has_definition ())
    item.get_definition ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::TraitItemConst &item)
{
  item.get_type ()->accept_vis (*this);

  if (item.has_expr ())
    item.get_expr ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::TraitItemType &item)
{}

void
EarlyNameResolver::visit (AST::Trait &trait)
{
  for (auto &item : trait.get_trait_items ())
    item->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::InherentImpl &impl)
{
  impl.get_type ()->accept_vis (*this);

  for (auto &generic : impl.get_generic_params ())
    generic->accept_vis (*this);

  for (auto &item : impl.get_impl_items ())
    item->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::TraitImpl &impl)
{
  impl.get_type ()->accept_vis (*this);

  for (auto &generic : impl.get_generic_params ())
    generic->accept_vis (*this);

  for (auto &item : impl.get_impl_items ())
    item->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::ExternalStaticItem &item)
{
  item.get_type ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::ExternalFunctionItem &item)
{
  for (auto &generic : item.get_generic_params ())
    generic->accept_vis (*this);

  for (auto &param : item.get_function_params ())
    param.get_type ()->accept_vis (*this);

  if (item.has_return_type ())
    item.get_return_type ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::ExternBlock &block)
{
  for (auto &item : block.get_extern_items ())
    item->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::MacroMatchFragment &match)
{}

void
EarlyNameResolver::visit (AST::MacroMatchRepetition &match)
{}

void
EarlyNameResolver::visit (AST::MacroMatcher &matcher)
{}

void
EarlyNameResolver::visit (AST::MacroRulesDefinition &rules_def)
{
  auto path = CanonicalPath::new_seg (rules_def.get_node_id (),
				      rules_def.get_rule_name ());
  resolver.get_macro_scope ().insert (path, rules_def.get_node_id (),
				      rules_def.get_locus ());
  mappings.insert_macro_def (&rules_def);
  rust_debug_loc (rules_def.get_locus (), "inserting macro def: [%s]",
		  path.get ().c_str ());
}

void
EarlyNameResolver::visit (AST::MacroInvocation &invoc)
{
  auto &invoc_data = invoc.get_invoc_data ();
  auto has_semicolon = invoc.has_semicolon ();

  // ??
  // switch on type of macro:
  //  - '!' syntax macro (inner switch)
  //      - procedural macro - "A token-based function-like macro"
  //      - 'macro_rules' (by example/pattern-match) macro? or not? "an
  // AST-based function-like macro"
  //      - else is unreachable
  //  - attribute syntax macro (inner switch)
  //  - procedural macro attribute syntax - "A token-based attribute
  // macro"
  //      - legacy macro attribute syntax? - "an AST-based attribute macro"
  //      - non-macro attribute: mark known
  //      - else is unreachable
  //  - derive macro (inner switch)
  //      - derive or legacy derive - "token-based" vs "AST-based"
  //      - else is unreachable
  //  - derive container macro - unreachable

  // lookup the rules for this macro
  NodeId resolved_node = UNKNOWN_NODEID;
  NodeId source_node = UNKNOWN_NODEID;
  if (has_semicolon)
    source_node = invoc.get_macro_node_id ();
  else
    source_node = invoc.get_pattern_node_id ();
  auto seg
    = CanonicalPath::new_seg (source_node, invoc_data.get_path ().as_string ());

  bool found = resolver.get_macro_scope ().lookup (seg, &resolved_node);
  if (!found)
    {
      rust_error_at (invoc.get_locus (), "unknown macro: [%s]",
		     seg.get ().c_str ());
      return;
    }

  // lookup the rules
  AST::MacroRulesDefinition *rules_def = nullptr;
  bool ok = mappings.lookup_macro_def (resolved_node, &rules_def);
  rust_assert (ok);

  mappings.insert_macro_invocation (invoc, rules_def);
}

// FIXME: ARTHUR: Do we need to resolve these as well here?

void
EarlyNameResolver::visit (AST::MetaItemPath &meta_item)
{}

void
EarlyNameResolver::visit (AST::MetaItemSeq &meta_item)
{}

void
EarlyNameResolver::visit (AST::MetaWord &meta_item)
{}

void
EarlyNameResolver::visit (AST::MetaNameValueStr &meta_item)
{}

void
EarlyNameResolver::visit (AST::MetaListPaths &meta_item)
{}

void
EarlyNameResolver::visit (AST::MetaListNameValueStr &meta_item)
{}

void
EarlyNameResolver::visit (AST::LiteralPattern &pattern)
{}

void
EarlyNameResolver::visit (AST::IdentifierPattern &pattern)
{
  if (pattern.has_pattern_to_bind ())
    pattern.get_pattern_to_bind ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::WildcardPattern &pattern)
{}

void
EarlyNameResolver::visit (AST::RangePatternBoundLiteral &bound)
{}

void
EarlyNameResolver::visit (AST::RangePatternBoundPath &bound)
{}

void
EarlyNameResolver::visit (AST::RangePatternBoundQualPath &bound)
{}

void
EarlyNameResolver::visit (AST::RangePattern &pattern)
{
  pattern.get_lower_bound ()->accept_vis (*this);
  pattern.get_upper_bound ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::ReferencePattern &pattern)
{
  pattern.get_referenced_pattern ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::StructPatternFieldTuplePat &field)
{
  field.get_index_pattern ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::StructPatternFieldIdentPat &field)
{
  field.get_ident_pattern ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::StructPatternFieldIdent &field)
{}

void
EarlyNameResolver::visit (AST::StructPattern &pattern)
{}

void
EarlyNameResolver::visit (AST::TupleStructItemsNoRange &tuple_items)
{
  for (auto &pattern : tuple_items.get_patterns ())
    pattern->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::TupleStructItemsRange &tuple_items)
{
  for (auto &pattern : tuple_items.get_lower_patterns ())
    pattern->accept_vis (*this);
  for (auto &pattern : tuple_items.get_upper_patterns ())
    pattern->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::TupleStructPattern &pattern)
{
  pattern.get_items ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::TuplePatternItemsMultiple &tuple_items)
{
  for (auto &pattern : tuple_items.get_patterns ())
    pattern->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::TuplePatternItemsRanged &tuple_items)
{
  for (auto &pattern : tuple_items.get_lower_patterns ())
    pattern->accept_vis (*this);
  for (auto &pattern : tuple_items.get_upper_patterns ())
    pattern->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::TuplePattern &pattern)
{
  pattern.get_items ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::GroupedPattern &pattern)
{
  pattern.get_pattern_in_parens ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::SlicePattern &pattern)
{
  for (auto &item : pattern.get_items ())
    item->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::EmptyStmt &stmt)
{}

void
EarlyNameResolver::visit (AST::LetStmt &stmt)
{
  if (stmt.has_type ())
    stmt.get_type ()->accept_vis (*this);

  if (stmt.has_init_expr ())
    stmt.get_init_expr ()->accept_vis (*this);

  stmt.get_pattern ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::ExprStmtWithoutBlock &stmt)
{
  stmt.get_expr ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::ExprStmtWithBlock &stmt)
{
  stmt.get_expr ()->accept_vis (*this);
}

void
EarlyNameResolver::visit (AST::TraitBound &bound)
{}

void
EarlyNameResolver::visit (AST::ImplTraitType &type)
{}

void
EarlyNameResolver::visit (AST::TraitObjectType &type)
{}

void
EarlyNameResolver::visit (AST::ParenthesisedType &type)
{}

void
EarlyNameResolver::visit (AST::ImplTraitTypeOneBound &type)
{}

void
EarlyNameResolver::visit (AST::TraitObjectTypeOneBound &type)
{}

void
EarlyNameResolver::visit (AST::TupleType &type)
{}

void
EarlyNameResolver::visit (AST::NeverType &type)
{}

void
EarlyNameResolver::visit (AST::RawPointerType &type)
{}

void
EarlyNameResolver::visit (AST::ReferenceType &type)
{}

void
EarlyNameResolver::visit (AST::ArrayType &type)
{}

void
EarlyNameResolver::visit (AST::SliceType &type)
{}

void
EarlyNameResolver::visit (AST::InferredType &type)
{}

void
EarlyNameResolver::visit (AST::BareFunctionType &type)
{
  for (auto &param : type.get_function_params ())
    param.get_type ()->accept_vis (*this);

  if (type.has_return_type ())
    type.get_return_type ()->accept_vis (*this);
}

} // namespace Resolver
} // namespace Rust
