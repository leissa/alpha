#pragma once

#include "util.h"

// This version includes §4.8: Using the Smaller Subtree

struct SExp;

/*
 * Pos
 */

struct Pos : public Dump<Pos> {
    Pos(int tag, Ptr<Pos>&& l, Ptr<Pos>&& r)
        : tag(tag)
        , l(std::move(l))
        , r(std::move(r)) {}

    std::string str() const;

    int tag;
    Ptr<Pos> l, r;
};

/// The Ptr<Pos> of free vars are owned by the VarMap.
/// Once a variable is bound ownership is transferred to the corresponding SLam.
using VarMap = std::unordered_map<std::string_view, Ptr<Pos>>;

/*
 * Exp
 */

struct Exp : public Dump<Exp> {
    virtual std::string str() const = 0;
    virtual Ptr<SExp> summarise(VarMap&) const = 0;
};

struct Var : public Exp {
    Var(std::string&& name)
        : name(std::move(name)) {}

    std::string str() const override;
    Ptr<SExp> summarise(VarMap&) const override;

    const std::string name;
};

struct Lam : public Exp {
    Lam(std::string name, Ptr<Exp>&& body)
        : name(std::move(name))
        , body(std::move(body)) {}

    std::string str() const override;
    Ptr<SExp> summarise(VarMap&) const override;

    std::string name;
    Ptr<Exp> body;
};

struct App : public Exp {
    App(Ptr<Exp>&& l, Ptr<Exp>&& r)
        : l(std::move(l))
        , r(std::move(r)) {}

    std::string str() const override;
    Ptr<SExp> summarise(VarMap&) const override;

    Ptr<Exp> l, r;
};

/*
 * SExp
 */

struct SExp : public Dump<SExp> {
    virtual std::string str() const = 0;
    virtual Ptr<Exp> rebuild(VarMap&) const = 0;
};

struct SVar : public SExp {
    std::string str() const override;
    Ptr<Exp> rebuild(VarMap&) const override;
};

struct SLam : public SExp {
    SLam(Ptr<Pos>&& tree, Ptr<SExp>&& body)
        : tree(std::move(tree))
        , body(std::move(body)) {}

    std::string str() const override;
    Ptr<Exp> rebuild(VarMap&) const override;

    Ptr<Pos> tree;
    Ptr<SExp> body;
};

struct SApp : public SExp {
    SApp(bool swap, Ptr<SExp>&& l, Ptr<SExp>&& r)
        : swap(swap)
        , l(std::move(l))
        , r(std::move(r)) {}

    std::string str() const override;
    Ptr<Exp> rebuild(VarMap&) const override;

    bool swap; ///< @c true, if bigger VarMap stems from l%eft.
    Ptr<SExp> l, r;
};
