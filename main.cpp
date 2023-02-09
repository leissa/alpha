#include "util.h"

/*
 * Pos
 */

struct Pos : public Dump<Pos> {
    Pos(Ptr<Pos>&& l, Ptr<Pos>&& r)
        : l(std::move(l))
        , r(std::move(r)) {}

    std::string str() const {
        if ( l &&  r) return "("s + l->str() + ", "s + r->str() + ")"s;
        if (!l &&  r) return "(o, " + r->str() + ")"s;
        if ( l && !r) return "("s + l->str() + ", o)"s;
        return "x"s;
    }

    Ptr<Pos> l, r;
};

/*
 * SExp
 */


/// The Ptr<Pos> of free vars are owned by the VarMap.
/// Once a variable is bound ownership is transferred to the corresponding SLam.

using VarMap = std::unordered_map<std::string_view, Ptr<Pos>>;

struct SExp : public Dump<SExp> {
    virtual std::string str() const = 0;
};

struct SVar : public SExp {
    std::string str() const override { return "x"s; }
};

struct SLam : public SExp {
    SLam(Ptr<Pos>&& tree, Ptr<SExp>&& body)
        : tree(std::move(tree))
        , body(std::move(body)) {}

    std::string str() const override { return "(lam "s + tree->str() + "."s + body->str() + ")"s; }

    Ptr<Pos> tree;
    Ptr<SExp> body;
};

struct SApp : public SExp {
    SApp(Ptr<SExp>&& l, Ptr<SExp>&& r)
        : l(std::move(l))
        , r(std::move(r)) {}

    std::string str() const override { return "("s + l->str() + " "s + r->str() + ")"s; }

    Ptr<SExp> l, r;
};

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

    std::string str() const override { return name; }

    Ptr<SExp> summarise(VarMap& vm) const override {
        auto [_, ins] = vm.emplace(name, mk<Pos>(nullptr, nullptr));
        assert(ins && "variable names must be unique");
        return mk<SVar>();
    }

    const std::string name;
};

struct Lam : public Exp {
    Lam(std::string name, Ptr<Exp>&& body)
        : name(std::move(name))
        , body(std::move(body)) {}

    std::string str() const override { return "(lam "s + name + "."s + body->str() + ")"s; }

    Ptr<SExp> summarise(VarMap& vm) const override {
        auto sbody = body->summarise(vm);
        if (auto i = vm.find(name); i != vm.end()) {
            auto tree = std::move(i->second);
            vm.erase(i);
            return mk<SLam>(std::move(tree), std::move(sbody));
        }
        return mk<SLam>(nullptr, std::move(sbody));
    }

    std::string name;
    Ptr<Exp> body;
};

struct App : public Exp {
    App(Ptr<Exp>&& l, Ptr<Exp>&& r)
        : l(std::move(l))
        , r(std::move(r)) {}

    std::string str() const override { return "("s + l->str() + " "s + r->str() + ")"s; }

    Ptr<SExp> summarise(VarMap& vml) const override {
        VarMap vmr;
        auto sl = l->summarise(vml);
        auto sr = r->summarise(vmr);

        for (auto& [_, tree] : vml)
            tree = mk<Pos>(std::move(tree), nullptr);

        for (auto& [name, r] : vmr) {
            auto [i, ins] = vml.try_emplace(name, nullptr);
            auto& tree = i->second;
            if (ins)
                tree = mk<Pos>(nullptr, std::move(r));
            else {
                assert(tree->l && !tree->r);
                tree->r = std::move(r);
            }
        }

        return mk<SApp>(std::move(sl), std::move(sr));
    }

    Ptr<Exp> l, r;
};

int main() {
    // lam x. (y x) x
    auto exp = mk<Lam>("x"s, mk<App>(mk<App>(mk<Var>("y"s), mk<Var>("x"s)), mk<Var>("x"s)));
    exp->dump();
    VarMap vm;
    auto sexp = exp->summarise(vm);
    sexp->dump();

    std::cout << "position of free vars:" << std::endl;
    for (auto&& [name, tree] : vm) {
        std::cout << name << ":" << std::endl;
        tree->dump();
    }
}
