#include "v2.h"

// This version includes §4.8: Using the Smaller Subtree

/*
 * str
 */

std::string Pos::str() const {
    if ( l &&  r) return "("s + l->str() + ", "s + r->str() + ")"s;
    if (!l &&  r) return "(o, " + r->str() + ")"s;
    if ( l && !r) return "("s + l->str() + ", o)"s;
    return "x"s;
}

std::string Var::str() const { return name; }
std::string Lam::str() const { return "(lam "s + name + "."s + body->str() + ")"s; }
std::string App::str() const { return "("s + l->str() + " "s + r->str() + ")"s; }

std::string SVar::str() const { return "x"s; }
std::string SLam::str() const { return "(lam "s + tree->str() + "."s + body->str() + ")"s; }
std::string SApp::str() const { return "("s + l->str() + " "s + r->str() + ")"s; }

/*
 * summarise
 */

Ptr<SExp> Var::summarise(VarMap& vm) const {
    auto [_, ins] = vm.emplace(name, mk<Pos>(0, nullptr, nullptr));
    assert(ins && "variable names must be unique");
    return mk<SVar>();
}


Ptr<SExp> Lam::summarise(VarMap& vm) const {
    auto sbody = body->summarise(vm);
    if (auto i = vm.find(name); i != vm.end()) {
        auto tree = std::move(i->second);
        vm.erase(i);
        return mk<SLam>(std::move(tree), std::move(sbody));
    }
    return mk<SLam>(nullptr, std::move(sbody));
}

Ptr<SExp> App::summarise(VarMap& vml) const {
    VarMap vmr;
    auto sl = l->summarise(vml);
    auto sr = r->summarise(vmr);

    // make sure vml is bigger
    bool swap = vml.size() < vmr.size();
    if (swap) std::swap(vml, vmr);

    for (auto& [name, rpos] : vmr) { // smaller vm
        auto [i, ins] = vml.try_emplace(name, nullptr);
        auto& lpos = i->second;
        if (ins) {
            lpos = mk<Pos>(rpos->tag + 1, nullptr, std::move(rpos));
        } else {
            assert(lpos->l && !lpos->r);
            lpos->r = std::move(rpos);
            lpos->tag = std::max(lpos->tag, lpos->r->tag + 1);
        }
    }

    return mk<SApp>(swap, std::move(sl), std::move(sr));
}

/*
 * rebuild
 */

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

