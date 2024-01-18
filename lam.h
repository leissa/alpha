#pragma once

#include "util.h"

// This version includes §4.8: Using the Smaller Subtree

struct SExp;

/*
 * Pos
 */

struct Pos : public Dump<Pos> {
#if ENABLE_SMALLER_SUBTREE
    Pos(int tag, SPtr<Pos> l, SPtr<Pos> r)
        : tag(tag)
        , l(l)
        , r(r) {}

    int tag;
#else
    Pos(SPtr<Pos> l, SPtr<Pos> r)
        : l(l)
        , r(r) {}
#endif

    std::string str() const;

    SPtr<Pos> l, r;
};

/// The SPtr<Pos> of free vars are owned by the VarMap.
/// Once a variable is bound ownership is transferred to the corresponding SLam.
using VarMap = std::unordered_map<std::string_view, SPtr<Pos>>;

/*
 * Exp
 */

struct Exp : public Dump<Exp> {
    virtual ~Exp() {}
    virtual std::string str() const = 0;
    virtual UPtr<SExp> summarise(VarMap&) const = 0;
};

struct Var : public Exp {
    Var(std::string&& name)
        : name(std::move(name)) {}

    std::string str() const override;
    UPtr<SExp> summarise(VarMap&) const override;

    const std::string name;
};

struct Lam : public Exp {
    Lam(std::string name, UPtr<Exp>&& body)
        : name(std::move(name))
        , body(std::move(body)) {}

    std::string str() const override;
    UPtr<SExp> summarise(VarMap&) const override;

    std::string name;
    UPtr<Exp> body;
};

struct App : public Exp {
    App(UPtr<Exp>&& l, UPtr<Exp>&& r)
        : l(std::move(l))
        , r(std::move(r)) {}

    std::string str() const override;
    UPtr<SExp> summarise(VarMap&) const override;

    UPtr<Exp> l, r;
};

/*
 * SExp
 */

struct SExp : public Dump<SExp> {
    virtual ~SExp() {}
    virtual std::string str() const = 0;
    virtual UPtr<Exp> rebuild(VarMap&) = 0;
};

struct SVar : public SExp {
    std::string str() const override;
    UPtr<Exp> rebuild(VarMap&) override;
};

struct SLam : public SExp {
    SLam(SPtr<Pos> pos, UPtr<SExp>&& body)
        : pos(pos)
        , sbody(std::move(body)) {}

    std::string str() const override;
    UPtr<Exp> rebuild(VarMap&) override;

    SPtr<Pos> pos;
    UPtr<SExp> sbody;
};

struct SApp : public SExp {
#if ENABLE_SMALLER_SUBTREE
    SApp(bool swap, UPtr<SExp> l, UPtr<SExp>&& r)
        : swap(swap)
        , sl(std::move(l))
        , sr(std::move(r)) {}

    bool swap; ///< @c true, if bigger VarMap stems from l%eft.
#else
    SApp(UPtr<SExp>&& l, UPtr<SExp>&& r)
        : sl(std::move(l))
        , sr(std::move(r)) {}
#endif

    std::string str() const override;
    UPtr<Exp> rebuild(VarMap&) override;

    UPtr<SExp> sl, sr;
};
