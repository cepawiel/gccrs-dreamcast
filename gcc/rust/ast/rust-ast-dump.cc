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

#include "rust-ast-dump.h"
#include "rust-diagnostics.h"

namespace Rust {
namespace AST {

Indent::Indent () : tabs (0) {}

std::ostream &
operator<< (std::ostream &stream, const Indent &indent)
{
  return stream << std::string (indent.tabs, '\t');
}

void
Indent::increment ()
{
  tabs++;
}

void
Indent::decrement ()
{
  rust_assert (tabs != 0);
  tabs--;
}

Dump::Dump (std::ostream &stream) : stream (stream), indentation (Indent ()) {}

void
Dump::go (AST::Crate &crate)
{
  visit_items_as_lines (crate.items, "");
}

void
Dump::go (AST::Item &item)
{
  item.accept_vis (*this);
}

template <typename T>
void
Dump::visit (std::unique_ptr<T> &node)
{
  node->accept_vis (*this);
}

template <typename T>
void
Dump::visit_items_joined_by_separator (T &collection,
				       const std::string &separator,
				       size_t start_offset, size_t end_offset)
{
  if (collection.size () > start_offset)
    {
      visit (collection.at (start_offset));
      auto size = collection.size () - end_offset;
      for (size_t i = start_offset + 1; i < size; i++)
	{
	  stream << separator;
	  visit (collection.at (i));
	}
    }
}

template <typename T>
void
Dump::visit_as_line (T &item, const std::string &trailing)
{
  stream << indentation;
  visit (item);
  stream << trailing << '\n';
}

template <typename T>
void
Dump::visit_items_as_lines (T &collection, const std::string &trailing)
{
  for (auto &item : collection)
    visit_as_line (item, trailing);
}

template <typename T>
void
Dump::visit_items_as_block (T &collection, const std::string &line_trailing,
			    char left_brace, char right_brace)
{
  if (collection.empty ())
    {
      stream << left_brace << right_brace << '\n';
    }
  else
    {
      stream << left_brace << '\n';

      indentation.increment ();
      visit_items_as_lines (collection, line_trailing);
      indentation.decrement ();

      stream << indentation << right_brace << '\n';
    }
}

void
Dump::visit (FunctionParam &param)
{
  visit (param.get_pattern ());
  stream << ": ";
  visit (param.get_type ());
}

void
Dump::visit (const Attribute &attrib)
{
  stream << "#[";
  visit_items_joined_by_separator (attrib.get_path ().get_segments (), "::");

  if (attrib.has_attr_input ())
    {
      stream << " = ";

      bool is_literal = attrib.get_attr_input ().get_attr_input_type ()
			== AST::AttrInput::AttrInputType::LITERAL;
      if (is_literal)
	{
	  auto &literal
	    = static_cast<AST::AttrInputLiteral &> (attrib.get_attr_input ());
	  const auto &value = literal.get_literal ().as_string ();

	  stream << "\"" << value << "\"";
	}
      else
	{
	  stream << "FIXME";
	}
    }

  stream << "]";
}

void
Dump::visit (const SimplePathSegment &segment)
{
  stream << segment.get_segment_name ();
}

void
Dump::visit (const Visibility &vis)
{
  switch (vis.get_vis_type ())
    {
    case Visibility::PUB:
      stream << "pub ";
      break;
    case Visibility::PUB_CRATE:
      stream << "pub(crate) ";
      break;
    case Visibility::PUB_SELF:
      stream << "pub(self) ";
      break;
    case Visibility::PUB_SUPER:
      stream << "pub(super) ";
      break;
    case Visibility::PUB_IN_PATH:
      stream << "pub(in " << vis.get_path ().as_string () << ") ";
      break;
    case Visibility::PRIV:
      break;
    }
}

void
Dump::visit (NamedFunctionParam &param)
{
  stream << param.get_name () << ": ";
  visit (param.get_type ());
}

void
Dump::visit (std::vector<std::unique_ptr<GenericParam>> &params)
{
  stream << "<";
  visit_items_joined_by_separator (params, ", ");
  stream << ">";
}

void
Dump::visit (TupleField &field)
{
  // TODO: do we need to emit outer attrs here?
  visit (field.get_visibility ());
  visit (field.get_field_type ());
}

void
Dump::visit (StructField &field)
{
  // TODO: do we need to emit outer attrs here?
  visit (field.get_visibility ());
  stream << field.get_field_name () << ": ";
  visit (field.get_field_type ());
}

void
Dump::visit (Token &tok)
{
  stream << tok.as_string ();
}

void
Dump::visit (DelimTokenTree &delim_tok_tree)
{
  indentation.increment ();
  stream << '\n' << indentation;

  auto tokens = delim_tok_tree.to_token_stream ();
  visit_items_joined_by_separator (tokens, " ");

  indentation.decrement ();
  stream << '\n' << indentation;
}

void
Dump::visit (AttrInputMetaItemContainer &input)
{}

void
Dump::visit (IdentifierExpr &ident_expr)
{
  stream << ident_expr.get_ident ();
}

void
Dump::visit (Lifetime &lifetime)
{}

void
Dump::visit (LifetimeParam &lifetime_param)
{}

void
Dump::visit (ConstGenericParam &lifetime_param)
{}

// rust-path.h
void
Dump::visit (PathInExpression &path)
{
  stream << path.as_string ();
}

void
Dump::visit (TypePathSegment &segment)
{}

void
Dump::visit (TypePathSegmentGeneric &segment)
{}

void
Dump::visit (TypePathSegmentFunction &segment)
{}

void
Dump::visit (TypePath &path)
{
  stream << path.as_string ();
}

void
Dump::visit (QualifiedPathInExpression &path)
{
  stream << path.as_string ();
}

void
Dump::visit (QualifiedPathInType &path)
{}

// rust-expr.h
void
Dump::visit (LiteralExpr &expr)
{
  stream << expr.as_string ();
}

void
Dump::visit (AttrInputLiteral &attr_input)
{}

void
Dump::visit (MetaItemLitExpr &meta_item)
{}

void
Dump::visit (MetaItemPathLit &meta_item)
{}

void
Dump::visit (BorrowExpr &expr)
{
  stream << '&';
  if (expr.get_is_double_borrow ())
    stream << '&';
  if (expr.get_is_mut ())
    stream << "mut ";

  visit (expr.get_borrowed_expr ());
}

void
Dump::visit (DereferenceExpr &expr)
{
  stream << '*';
  visit (expr.get_dereferenced_expr ());
}

void
Dump::visit (ErrorPropagationExpr &expr)
{
  visit (expr.get_propagating_expr ());
  stream << '?';
}

void
Dump::visit (NegationExpr &expr)
{
  switch (expr.get_expr_type ())
    {
    case NegationOperator::NEGATE:
      stream << '-';
      break;
    case NegationOperator::NOT:
      stream << '!';
      break;
    }
  visit (expr.get_negated_expr ());
}

void
Dump::visit (ArithmeticOrLogicalExpr &expr)
{
  auto op = "";
  switch (expr.get_expr_type ())
    {
    case ArithmeticOrLogicalOperator::ADD:
      op = "+";
      break;

    case ArithmeticOrLogicalOperator::SUBTRACT:
      op = "-";
      break;

    case ArithmeticOrLogicalOperator::MULTIPLY:
      op = "*";
      break;

    case ArithmeticOrLogicalOperator::DIVIDE:
      op = "/";
      break;

    case ArithmeticOrLogicalOperator::MODULUS:
      op = "%";
      break;

    case ArithmeticOrLogicalOperator::BITWISE_AND:
      op = "&";
      break;

    case ArithmeticOrLogicalOperator::BITWISE_OR:
      op = "|";
      break;

    case ArithmeticOrLogicalOperator::BITWISE_XOR:
      op = "^";
      break;

    case ArithmeticOrLogicalOperator::LEFT_SHIFT:
      op = "<<";
      break;

    case ArithmeticOrLogicalOperator::RIGHT_SHIFT:
      op = ">>";
      break;
    }

  visit (expr.get_left_expr ());
  stream << " " << op << " ";
  visit (expr.get_right_expr ());
}

void
Dump::visit (ComparisonExpr &expr)
{
  auto op = "";
  switch (expr.get_expr_type ())
    {
    case ComparisonOperator::EQUAL:
      op = "==";
      break;
    case ComparisonOperator::NOT_EQUAL:
      op = "!=";
      break;

    case ComparisonOperator::GREATER_THAN:
      op = ">";
      break;

    case ComparisonOperator::LESS_THAN:
      op = "<";
      break;

    case ComparisonOperator::GREATER_OR_EQUAL:
      op = ">=";
      break;

    case ComparisonOperator::LESS_OR_EQUAL:
      op = "<=";
      break;
    }

  visit (expr.get_left_expr ());
  stream << " " << op << " ";
  visit (expr.get_right_expr ());
}

void
Dump::visit (LazyBooleanExpr &expr)
{
  auto op = "";
  switch (expr.get_expr_type ())
    {
    case LazyBooleanOperator::LOGICAL_AND:
      op = "&&";
      break;
    case LazyBooleanOperator::LOGICAL_OR:
      op = "||";
      break;
    }

  visit (expr.get_left_expr ());
  stream << " " << op << " ";
  visit (expr.get_right_expr ());
}

void
Dump::visit (TypeCastExpr &expr)
{
  visit (expr.get_casted_expr ());
  stream << " as ";
  visit (expr.get_type_to_cast_to ());
}

void
Dump::visit (AssignmentExpr &expr)
{
  expr.visit_lhs (*this);
  stream << " = ";
  expr.visit_rhs (*this);
}

void
Dump::visit (CompoundAssignmentExpr &expr)
{
  auto op = "";
  switch (expr.get_expr_type ())
    {
    case CompoundAssignmentOperator::ADD:
      op = "+";
      break;

    case CompoundAssignmentOperator::SUBTRACT:
      op = "-";
      break;

    case CompoundAssignmentOperator::MULTIPLY:
      op = "*";
      break;

    case CompoundAssignmentOperator::DIVIDE:
      op = "/";
      break;

    case CompoundAssignmentOperator::MODULUS:
      op = "%";
      break;

    case CompoundAssignmentOperator::BITWISE_AND:
      op = "&";
      break;

    case CompoundAssignmentOperator::BITWISE_OR:
      op = "|";
      break;

    case CompoundAssignmentOperator::BITWISE_XOR:
      op = "^";
      break;

    case CompoundAssignmentOperator::LEFT_SHIFT:
      op = "<<";
      break;

    case CompoundAssignmentOperator::RIGHT_SHIFT:
      op = ">>";
      break;
    }

  visit (expr.get_left_expr ());
  stream << " " << op << "= ";
  visit (expr.get_right_expr ());
}

void
Dump::visit (GroupedExpr &expr)
{
  stream << '(';
  visit (expr.get_expr_in_parens ());
  stream << ')';
}

void
Dump::visit (ArrayElemsValues &elems)
{
  visit_items_joined_by_separator (elems.get_values (), ", ");
}

void
Dump::visit (ArrayElemsCopied &elems)
{
  visit (elems.get_elem_to_copy ());
  stream << "; ";
  visit (elems.get_num_copies ());
}

void
Dump::visit (ArrayExpr &expr)
{
  stream << '[';
  visit (expr.get_array_elems ());
  stream << ']';
}

void
Dump::visit (ArrayIndexExpr &expr)
{
  visit (expr.get_array_expr ());
  stream << '[';
  visit (expr.get_index_expr ());
  stream << ']';
}

void
Dump::visit (TupleExpr &expr)
{}

void
Dump::visit (TupleIndexExpr &expr)
{}

void
Dump::visit (StructExprStruct &expr)
{}

void
Dump::visit (StructExprFieldIdentifier &field)
{}

void
Dump::visit (StructExprFieldIdentifierValue &field)
{}

void
Dump::visit (StructExprFieldIndexValue &field)
{}

void
Dump::visit (StructExprStructFields &expr)
{}

void
Dump::visit (StructExprStructBase &expr)
{}

void
Dump::visit (CallExpr &expr)
{
  visit (expr.get_function_expr ());

  stream << '(' << '\n';
  indentation.increment ();

  visit_items_as_lines (expr.get_params (), ",");

  indentation.decrement ();
  stream << indentation << ')';
}

void
Dump::visit (MethodCallExpr &expr)
{}

void
Dump::visit (FieldAccessExpr &expr)
{}

void
Dump::visit (ClosureExprInner &expr)
{}

void
Dump::visit (BlockExpr &expr)
{
  stream << "{\n";
  indentation.increment ();

  visit_items_as_lines (expr.get_statements (), "; /* stmt */");

  if (expr.has_tail_expr ())
    visit_as_line (expr.get_tail_expr (), " /* tail expr */\n");

  indentation.decrement ();
  stream << indentation << "}\n";
}

void
Dump::visit (ClosureExprInnerTyped &expr)
{}

void
Dump::visit (ContinueExpr &expr)
{}

void
Dump::visit (BreakExpr &expr)
{}

void
Dump::visit (RangeFromToExpr &expr)
{
  visit (expr.get_from_expr ());
  stream << "..";
  visit (expr.get_to_expr ());
}

void
Dump::visit (RangeFromExpr &expr)
{
  visit (expr.get_from_expr ());
  stream << "..";
}

void
Dump::visit (RangeToExpr &expr)
{
  stream << "..";
  visit (expr.get_to_expr ());
}

void
Dump::visit (RangeFullExpr &expr)
{
  stream << "..";
}

void
Dump::visit (RangeFromToInclExpr &expr)
{
  visit (expr.get_from_expr ());
  stream << "..=";
  visit (expr.get_to_expr ());
}

void
Dump::visit (RangeToInclExpr &expr)
{
  stream << "..=";
  visit (expr.get_to_expr ());
}

void
Dump::visit (ReturnExpr &expr)
{}

void
Dump::visit (UnsafeBlockExpr &expr)
{}

void
Dump::visit (LoopExpr &expr)
{}

void
Dump::visit (WhileLoopExpr &expr)
{}

void
Dump::visit (WhileLetLoopExpr &expr)
{}

void
Dump::visit (ForLoopExpr &expr)
{}

void
Dump::visit (IfExpr &expr)
{
  stream << "if ";
  visit (expr.get_condition_expr ());
  stream << " ";
  visit (expr.get_if_block ());
}

void
Dump::visit (IfExprConseqElse &expr)
{
  stream << "if ";
  visit (expr.get_condition_expr ());
  stream << " ";
  visit (expr.get_if_block ());
  stream << indentation << "else ";
  visit (expr.get_else_block ());
}

void
Dump::visit (IfExprConseqIf &expr)
{
  stream << "if ";
  visit (expr.get_condition_expr ());
  stream << " ";
  visit (expr.get_if_block ());
  stream << indentation << "else ";
  // The "if" part of the "else if" is printed by the next visitor
  visit (expr.get_conseq_if_expr ());
}

void
Dump::visit (IfExprConseqIfLet &expr)
{}

void
Dump::visit (IfLetExpr &expr)
{}

void
Dump::visit (IfLetExprConseqElse &expr)
{}

void
Dump::visit (IfLetExprConseqIf &expr)
{}

void
Dump::visit (IfLetExprConseqIfLet &expr)
{}

void
Dump::visit (MatchExpr &expr)
{}

void
Dump::visit (AwaitExpr &expr)
{}

void
Dump::visit (AsyncBlockExpr &expr)
{}

// rust-item.h
void
Dump::visit (TypeParam &param)
{
  stream << param.get_type_representation ();
  if (param.has_type ())
    {
      stream << " = ";
      visit (param.get_type ());
    }
}

void
Dump::visit (LifetimeWhereClauseItem &item)
{}

void
Dump::visit (TypeBoundWhereClauseItem &item)
{}

void
Dump::visit (Method &method)
{
  // FIXME: Do we really need to dump the indentation here?
  stream << indentation;
  visit (method.get_visibility ());
  stream << "fn " << method.get_method_name () << '(';

  stream << method.get_self_param ().as_string () << ", ";
  visit_items_joined_by_separator (method.get_function_params (), ", ");

  stream << ") ";

  if (method.has_return_type ())
    {
      stream << "-> ";
      visit (method.get_return_type ());
      stream << " ";
    }

  auto &block = method.get_definition ();
  if (!block)
    stream << ';';
  else
    visit (block);

  stream << '\n';
}

void
Dump::visit (Module &module)
{
  //  Syntax:
  //	mod IDENTIFIER ;
  //     | mod IDENTIFIER {
  //	  InnerAttribute*
  //	  Item*
  //	}

  visit (module.get_visibility ());
  stream << "mod " << module.get_name ();

  if (module.get_kind () == Module::UNLOADED)
    {
      stream << ";\n";
    }
  else /* Module::LOADED */
    {
      stream << " {\n";

      indentation.increment ();

      visit_items_as_lines (module.get_inner_attrs ());
      visit_items_as_lines (module.get_items ());

      indentation.decrement ();

      stream << indentation << "}\n";
    }
}

void
Dump::visit (ExternCrate &crate)
{}

void
Dump::visit (UseTreeGlob &use_tree)
{}

void
Dump::visit (UseTreeList &use_tree)
{}

void
Dump::visit (UseTreeRebind &use_tree)
{}

void
Dump::visit (UseDeclaration &use_decl)
{}

void
Dump::visit (Function &function)
{
  visit (function.get_visibility ());

  stream << "fn " << function.get_function_name ();
  if (function.has_generics ())
    visit (function.get_generic_params ());

  stream << '(';
  visit_items_joined_by_separator (function.get_function_params ());
  stream << ") ";

  if (function.has_return_type ())
    {
      stream << "-> ";
      visit (function.get_return_type ());
      stream << " ";
    }

  auto &block = function.get_definition ();
  if (!block)
    stream << ';';
  else
    visit (block);

  stream << '\n';
}

void
Dump::visit (TypeAlias &type_alias)
{
  // Syntax:
  // Visibility? type IDENTIFIER GenericParams? WhereClause? = Type;

  // Note: Associated types are handled by `AST::TraitItemType`.

  if (type_alias.has_visibility ())
    visit (type_alias.get_visibility ());
  stream << "type " << type_alias.get_new_type_name ();
  if (type_alias.has_generics ())
    visit (type_alias.get_generic_params ());
  if (type_alias.has_where_clause ())
    {
    } // FIXME: WhereClause
  stream << " = ";
  visit (type_alias.get_type_aliased ());
  stream << ";\n";
}

void
Dump::visit (StructStruct &struct_item)
{
  stream << "struct " << struct_item.get_identifier ();
  if (struct_item.has_generics ())
    visit (struct_item.get_generic_params ());

  // FIXME: where-clause

  if (struct_item.is_unit_struct ())
    stream << ";\n";
  else
    visit_items_as_block (struct_item.get_fields (), ",");
}

void
Dump::visit (TupleStruct &tuple_struct)
{
  stream << "struct " << tuple_struct.get_identifier ();
  if (tuple_struct.has_generics ())
    visit (tuple_struct.get_generic_params ());

  // FIXME: where-clause

  stream << '(';
  visit_items_joined_by_separator (tuple_struct.get_fields (), ", ");
  stream << ");\n";
}

void
Dump::visit (EnumItem &item)
{
  stream << item.get_identifier ();
}

void
Dump::visit (EnumItemTuple &item)
{
  stream << item.get_identifier () << '(';
  visit_items_joined_by_separator (item.get_tuple_fields (), ", ");
  stream << ')';
}

void
Dump::visit (EnumItemStruct &item)
{
  stream << item.get_identifier ();
  visit_items_as_block (item.get_struct_fields (), ",");
}

void
Dump::visit (EnumItemDiscriminant &item)
{
  stream << item.get_identifier () << " = ";
  visit (item.get_expr ());
}

void
Dump::visit (Enum &enum_item)
{
  stream << "enum " << enum_item.get_identifier ();
  if (enum_item.has_generics ())
    visit (enum_item.get_generic_params ());

  // FIXME: where-clause

  visit_items_as_block (enum_item.get_variants (), ",");
}

void
Dump::visit (Union &union_item)
{
  stream << "union " << union_item.get_identifier ();
  if (union_item.has_generics ())
    visit (union_item.get_generic_params ());

  // FIXME: where-clause

  visit_items_as_block (union_item.get_variants (), ",");
}

void
Dump::visit (ConstantItem &const_item)
{}

void
Dump::visit (StaticItem &static_item)
{}

void
Dump::visit_function_common (std::unique_ptr<Type> &return_type,
			     std::unique_ptr<BlockExpr> &block)
{
  // FIXME: This should format the `<vis> fn <name> ( [args] )` as well
  if (return_type)
    {
      stream << "-> ";
      visit (return_type);
    }

  if (block)
    {
      if (return_type)
	{
	  stream << ' ';
	  visit (block);
	}
    }
  else
    stream << ";\n";
}

void
Dump::visit (TraitItemFunc &item)
{
  auto func = item.get_trait_function_decl ();
  stream << indentation << "fn " << func.get_identifier () << '(';

  visit_items_joined_by_separator (func.get_function_params ());

  stream << ") ";

  visit_function_common (func.get_return_type (), item.get_definition ());
}

void
Dump::visit (TraitItemMethod &item)
{
  auto method = item.get_trait_method_decl ();

  // FIXME: Do we really need to dump the indentation here?
  stream << indentation;

  // FIXME: Can we have visibility here?
  // emit_visibility (method.get_visibility ());
  stream << "fn " << method.get_identifier () << '(';

  stream << method.get_self_param ().as_string () << ", ";

  visit_items_joined_by_separator (method.get_function_params (), ", ");

  stream << ") ";

  visit_function_common (method.get_return_type (), item.get_definition ());
}

void
Dump::visit (TraitItemConst &item)
{
  stream << indentation << "const " << item.get_identifier () << ": ";
  visit (item.get_type ());
  stream << ";\n";
}

void
Dump::visit (TraitItemType &item)
{
  stream << indentation << "type " << item.get_identifier () << ";\n";
}

void
Dump::visit (Trait &trait)
{
  for (const auto &attr : trait.get_outer_attrs ())
    {
      visit (attr);
      stream << "\n" << indentation;
    }

  visit (trait.get_visibility ());

  stream << "trait " << trait.get_identifier ();

  // Traits actually have an implicit Self thrown at the start, so we must
  // expect the number of generic params to be > 1
  if (trait.get_generic_params ().size () > 1)
    {
      stream << "<";
      visit_items_joined_by_separator (trait.get_generic_params (), ", ", 1);
      stream << ">";
    }

  visit_items_as_block (trait.get_trait_items (), "");
}

void
Dump::visit (InherentImpl &impl)
{
  stream << "impl ";

  // FIXME: Handle generics

  visit (impl.get_type ());

  // FIXME: Handle where-clause
  // FIXME: Handle inner attributes

  visit_items_as_block (impl.get_impl_items (), "");
}

void
Dump::visit (TraitImpl &impl)
{
  stream << "impl ";
  visit (impl.get_trait_path ());
  stream << " for ";
  visit (impl.get_type ());
  stream << " {\n";

  indentation.increment ();

  for (auto &item : impl.get_impl_items ())
    {
      stream << indentation;
      visit (item);
    }

  indentation.decrement ();
  stream << "\n}\n";
}

void
Dump::visit (ExternalStaticItem &item)
{}

void
Dump::visit (ExternalFunctionItem &function)
{
  visit (function.get_visibility ());

  stream << "fn " << function.get_identifier () << '(';

  visit_items_joined_by_separator (function.get_function_params ());

  stream << ')';
  if (function.has_return_type ())
    {
      stream << "-> ";
      visit (function.get_return_type ());
    }
}

void
Dump::visit (ExternBlock &block)
{
  stream << "extern ";

  if (block.has_abi ())
    stream << "\"" << block.get_abi () << "\" ";

  visit_items_as_block (block.get_extern_items (), ";");
}

static std::pair<char, char>
get_delimiters (DelimType delim)
{
  auto start_delim = '\0';
  auto end_delim = '\0';

  switch (delim)
    {
    case PARENS:
      start_delim = '(';
      end_delim = ')';
      break;
    case SQUARE:
      start_delim = '[';
      end_delim = ']';
      break;
    case CURLY:
      start_delim = '{';
      end_delim = '}';
      break;
    }

  return {start_delim, end_delim};
}

void
Dump::visit (MacroMatchFragment &match)
{
  stream << '$' << match.get_ident () << ':'
	 << match.get_frag_spec ().as_string ();
}

void
Dump::visit (MacroMatchRepetition &repetition)
{
  stream << "$(";

  visit_items_joined_by_separator (repetition.get_matches (), " ");

  auto op_char = '\0';
  switch (repetition.get_op ())
    {
    case MacroMatchRepetition::ANY:
      op_char = '*';
      break;
    case MacroMatchRepetition::ONE_OR_MORE:
      op_char = '+';
      break;
    case MacroMatchRepetition::ZERO_OR_ONE:
      op_char = '?';
      break;
    case MacroMatchRepetition::NONE:
      break;
    }

  stream << ')';

  if (repetition.has_sep ())
    stream << repetition.get_sep ()->as_string ();

  stream << op_char;
}

void
Dump::visit (MacroMatcher &matcher)
{
  auto delimiters = get_delimiters (matcher.get_delim_type ());

  stream << delimiters.first;

  visit_items_joined_by_separator (matcher.get_matches (), " ");

  stream << delimiters.second;
}

void
Dump::visit (MacroRule &rule)
{
  visit (rule.get_matcher ());
  stream << " => ";
  visit (rule.get_transcriber ().get_token_tree ());
  stream << ";";
}

void
Dump::visit (MacroRulesDefinition &rules_def)
{
  for (auto &outer_attr : rules_def.get_outer_attrs ())
    visit (outer_attr);

  stream << "macro_rules! " << rules_def.get_rule_name ();

  visit_items_as_block (rules_def.get_rules (), ";");
}

void
Dump::visit (MacroInvocation &macro_invoc)
{}

void
Dump::visit (MetaItemPath &meta_item)
{}

void
Dump::visit (MetaItemSeq &meta_item)
{}

void
Dump::visit (MetaWord &meta_item)
{}

void
Dump::visit (MetaNameValueStr &meta_item)
{}

void
Dump::visit (MetaListPaths &meta_item)
{}

void
Dump::visit (MetaListNameValueStr &meta_item)
{}

// rust-pattern.h
void
Dump::visit (LiteralPattern &pattern)
{}

void
Dump::visit (IdentifierPattern &pattern)
{
  stream << pattern.get_ident ();
}

void
Dump::visit (WildcardPattern &pattern)
{}

// void Dump::visit(RangePatternBound& bound){}

void
Dump::visit (RangePatternBoundLiteral &bound)
{}

void
Dump::visit (RangePatternBoundPath &bound)
{}

void
Dump::visit (RangePatternBoundQualPath &bound)
{}

void
Dump::visit (RangePattern &pattern)
{}

void
Dump::visit (ReferencePattern &pattern)
{}

// void Dump::visit(StructPatternField& field){}

void
Dump::visit (StructPatternFieldTuplePat &field)
{}

void
Dump::visit (StructPatternFieldIdentPat &field)
{}

void
Dump::visit (StructPatternFieldIdent &field)
{}

void
Dump::visit (StructPattern &pattern)
{}

// void Dump::visit(TupleStructItems& tuple_items){}

void
Dump::visit (TupleStructItemsNoRange &tuple_items)
{}

void
Dump::visit (TupleStructItemsRange &tuple_items)
{}

void
Dump::visit (TupleStructPattern &pattern)
{}

// void Dump::visit(TuplePatternItems& tuple_items){}

void
Dump::visit (TuplePatternItemsMultiple &tuple_items)
{}

void
Dump::visit (TuplePatternItemsRanged &tuple_items)
{}

void
Dump::visit (TuplePattern &pattern)
{}

void
Dump::visit (GroupedPattern &pattern)
{}

void
Dump::visit (SlicePattern &pattern)
{}

// rust-stmt.h
void
Dump::visit (EmptyStmt &stmt)
{}

void
Dump::visit (LetStmt &stmt)
{
  stream << "let ";
  auto &pattern = stmt.get_pattern ();
  if (pattern)
    visit (pattern);

  if (stmt.has_type ())
    {
      stream << ": ";
      visit (stmt.get_type ());
    }

  if (stmt.has_init_expr ())
    {
      stream << " = ";
      visit (stmt.get_init_expr ());
    }
}

void
Dump::visit (ExprStmtWithoutBlock &stmt)
{
  visit (stmt.get_expr ());
}

void
Dump::visit (ExprStmtWithBlock &stmt)
{
  visit (stmt.get_expr ());
}

// rust-type.h
void
Dump::visit (TraitBound &bound)
{}

void
Dump::visit (ImplTraitType &type)
{}

void
Dump::visit (TraitObjectType &type)
{}

void
Dump::visit (ParenthesisedType &type)
{}

void
Dump::visit (ImplTraitTypeOneBound &type)
{}

void
Dump::visit (TraitObjectTypeOneBound &type)
{}

void
Dump::visit (TupleType &type)
{}

void
Dump::visit (NeverType &type)
{}

void
Dump::visit (RawPointerType &type)
{}

void
Dump::visit (ReferenceType &type)
{
  visit (type.get_type_referenced ());
}

void
Dump::visit (ArrayType &type)
{
  visit (type.get_elem_type ());
}

void
Dump::visit (SliceType &type)
{
  visit (type.get_elem_type ());
}

void
Dump::visit (InferredType &type)
{
  stream << "_";
}

void
Dump::visit (BareFunctionType &type)
{}

} // namespace AST
} // namespace Rust
