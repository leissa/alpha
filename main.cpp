#include <cassert>

#include <ios>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
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
    Tree(std::unique_ptr<Tree>&& l, std::unique_ptr<Tree>&& r)
        : l(std::move(l))
        , r(std::move(r)) {}

    Tree(Tree&& other) {
        swap(*this, other);
    }

    bool here() const { return !l && !r; }

    std::string str() const {
        if ( l &&  r) return "("s + l->str() + ", "s + r->str() + ")"s;
        if (!l &&  r) return "(o, " + r->str() + ")"s;
        if ( l && !r) return "("s + l->str() + ", o)"s;
        assert(here());
        return "x"s;
    }

    void dump() const { std::cout << str() << std::endl; }

    friend void swap(Tree& p1, Tree& p2) {
        using std::swap;
        swap(p1.l, p2.l);
        swap(p1.r, p2.r);
    }

    std::unique_ptr<Tree> l;
    std::unique_ptr<Tree> r;
};

using Ptr = std::unique_ptr<Tree>;

Ptr here() { return std::make_unique<Tree>(nullptr, nullptr); }
Ptr l(Ptr&& l) { return std::make_unique<Tree>(std::move(l), nullptr); }
Ptr r(Ptr&& r) { return std::make_unique<Tree>(nullptr, std::move(r)); }
Ptr both(Ptr&& l, Ptr&& r) { return std::make_unique<Tree>(std::move(l), std::move(r)); }

template<bool L>
Ptr one(Ptr&& p) { return L ? l(std::move(p)) : r(std::move(p)); }

}

/*
 * SExp
 */


/// The pos::Ptr of free vars are owned by the VarMap.
/// Once a variable is bound ownership is transferred to the corresponding SLam.

using VarMap = std::unordered_map<std::string_view, pos::Ptr>;

struct SExp {
    virtual std::string str() const = 0;
    void dump() const { std::cout << str() << std::endl; }
};

struct SVar : public SExp {
    std::string str() const override { return "!"s; }
};

struct SLam : public SExp {
    SLam(pos::Ptr&& tree, std::unique_ptr<const SExp>&& body)
        : tree(std::move(tree))
        , body(std::move(body)) {}

    std::string str() const override { return "(lam "s + tree->str() + "."s + body->str() + ")"s; }

    pos::Ptr tree;
    std::unique_ptr<const SExp> body;
};

struct SApp : public SExp {
    SApp(std::unique_ptr<const SExp>&& l, std::unique_ptr<const SExp>&& r)
        : l(std::move(l))
        , r(std::move(r)) {}

    std::string str() const override { return "("s + l->str() + " "s + r->str() + ")"s; }

    std::unique_ptr<const SExp> l;
    std::unique_ptr<const SExp> r;
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

    std::unique_ptr<const SExp> summarise(VarMap& vm) const override {
        auto [_, ins] = vm.emplace(name, pos::here());
        assert(ins && "variable names must be unique");
        return std::make_unique<const SVar>();
    }

    const std::string name;
};

struct Lam : public Exp {
    Lam(std::string name, std::unique_ptr<const Exp>&& body)
        : name(std::move(name))
        , body(std::move(body)) {}

    std::string str() const override { return "(lam "s + name + "."s + body->str() + ")"s; }

    std::unique_ptr<const SExp> summarise(VarMap& vm) const override {
        auto sbody = body->summarise(vm);
        if (auto i = vm.find(name); i != vm.end()) {
            auto tree = std::move(i->second);
            vm.erase(i);
            return std::make_unique<const SLam>(std::move(tree), std::move(sbody));
        }
        return std::make_unique<const SLam>(nullptr, std::move(sbody));
    }

    std::string name;
    std::unique_ptr<const Exp> body;
};

struct App : public Exp {
    App(std::unique_ptr<const Exp>&& l, std::unique_ptr<const Exp>&& r)
        : l(std::move(l))
        , r(std::move(r)) {}

    std::string str() const override { return "("s + l->str() + " "s + r->str() + ")"s; }

    std::unique_ptr<const SExp> summarise(VarMap& vml) const override {
        VarMap vmr;
        auto sl = l->summarise(vml);
        auto sr = r->summarise(vmr);

        for (auto& [_, tree] : vml)
            tree = pos::l(std::move(tree));

        for (auto& [name, r] : vmr) {
            auto [i, ins] = vml.try_emplace(name, nullptr);
            auto& tree = i->second;
            if (ins)
                tree = pos::r(std::move(r));
            else {
                assert(tree->l && !tree->r);
                tree->r = std::move(r);
            }
        }

        return std::make_unique<const SApp>(std::move(sl), std::move(sr));
    }

    std::unique_ptr<const Exp> l;
    std::unique_ptr<const Exp> r;
};

int main() {
    // (f x) x
    auto exp = std::make_unique<const App>(std::make_unique<const App>(std::make_unique<const Var>("f"s), std::make_unique<const Var>("x"s)), std::make_unique<const Var>("x"s));
    // pos tree for x
    auto pos = pos::both(pos::r(pos::here()), pos::here());
    exp->dump();
    pos->dump();
    VarMap vm;
    auto sexp = exp->summarise(vm);
    sexp->dump();
    for (auto&& [name, tree] : vm) {
        std::cout << name << ":" << std::endl;
        tree->dump();
    }
}
