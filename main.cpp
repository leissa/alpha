#include <cassert>

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

using namespace std::string_literals;

#include "hash.h"

/*
 * Pos
 */

namespace pos {

struct Tree {
    Tree() = default;
    Tree(std::unique_ptr<const Tree>&& l, std::unique_ptr<const Tree>&& r)
        : l(std::move(l))
        , r(std::move(r)) {}

    Tree(Tree&& other) {
        swap(*this, other);
    }

    bool here() const { return !l && !r; }

    std::string str() const {
        if ( l &&  r) return "("s + l->str() + ", "s + r->str() + ")"s;
        if (!l &&  r) return "(o, " + r->str() + ")"s;
        if ( l && !r) return "("s + l->str() + ", x)"s;
        assert(here());
        return "!"s;
    }

    void dump() const { std::cout << str() << std::endl; }

    friend void swap(Tree& p1, Tree& p2) {
        using std::swap;
        swap(p1.l, p2.l);
        swap(p1.r, p2.r);
    }

    std::unique_ptr<const Tree> l;
    std::unique_ptr<const Tree> r;
};

using Ptr = std::unique_ptr<const Tree>;

Ptr here() { return std::make_unique<const Tree>(nullptr, nullptr); }
Ptr l(Ptr&& l) { return std::make_unique<const Tree>(std::move(l), nullptr); }
Ptr r(Ptr&& r) { return std::make_unique<const Tree>(nullptr, std::move(r)); }
Ptr both(Ptr&& l, Ptr&& r) { return std::make_unique<const Tree>(std::move(l), std::move(r)); }

}

/*
 * SExp
 */

struct SExp {
};

struct SVar : public SExp {
};

struct SLam : public SExp {
    SLam(std::unique_ptr<const pos::Tree>&& shape, std::unique_ptr<const SExp>&& body)
        : shape(std::move(shape))
        , body(std::move(body)) {}

    std::unique_ptr<const pos::Tree> shape;
    std::unique_ptr<const SExp> body;
};

struct SApp : public SExp {
    SApp(std::unique_ptr<const SExp>&& callee, std::unique_ptr<const SExp>&& arg)
        : callee(std::move(callee))
        , arg(std::move(arg)) {}

    std::unique_ptr<const SExp> callee;
    std::unique_ptr<const SExp> arg;
};

using VarMap = std::unordered_map<std::string, pos::Tree*>;

struct ESummary {
    SExp sexp;
    VarMap vm;
};

/*
 * Exp
 */

struct Exp {
    void dump() const { std::cout << str() << std::endl; }
    virtual std::string str() const = 0;
    virtual std::unique_ptr<const SExp> summarise(VarMap&) const = 0;
};

struct Var : public Exp {
    Var(std::string&& name)
        : name(std::move(name)) {}

    std::string str() const override { return name; }
    std::unique_ptr<const SExp> summarise(VarMap&) const override {
        return {};
    }

    const std::string name;
};

struct Lam : public Exp {
    Lam(std::string name, std::unique_ptr<const Exp>&& body)
        : name(std::move(name))
        , body(std::move(body)) {}

    std::string str() const override { return "(lam "s + name + "."s + body->str() + ")"s; }

    std::unique_ptr<const SExp> summarise(VarMap&) const override {
        return {};
    }

    std::string name;
    std::unique_ptr<const Exp> body;
};

struct App : public Exp {
    App(std::unique_ptr<const Exp>&& callee, std::unique_ptr<const Exp>&& arg)
        : callee(std::move(callee))
        , arg(std::move(arg)) {}

    std::string str() const override { return "("s + callee->str() + " "s + arg->str() + ")"s; }

    std::unique_ptr<const SExp> summarise(VarMap&) const override {
        return {};
    }

    std::unique_ptr<const Exp> callee;
    std::unique_ptr<const Exp> arg;
};

int main() {
    // (f x) x
    auto expr = std::make_unique<const App>(std::make_unique<const App>(std::make_unique<const Var>("f"s), std::make_unique<const Var>("x"s)), std::make_unique<const Var>("x"s));
    // pos tree for x
    auto pos = pos::both(pos::r(pos::here()), pos::here());
    expr->dump();
    pos->dump();
    //auto ptree = pt_both(pt_r(pt_here()), pt_here());
}
