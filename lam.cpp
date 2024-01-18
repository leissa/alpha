#include "lam.h"

// This version includes §4.8: Using the Smaller Subtree

/*
 * str
 */

std::string Pos::str() const {
    if ( l &&  r) return "("s + l->str() + ", "s + r->str() + ")"s;
    if (!l &&  r) return "(o, " + r->str() + ")"s;
    if ( l && !r) return "("s + l->str() + ", o)"s;
    return "!"s;
}

std::string Var::str() const { return name; }
std::string Lam::str() const { return "(lam "s + name + "."s + body->str() + ")"s; }
std::string App::str() const { return "("s + l->str() + " "s + r->str() + ")"s; }

std::string SVar::str() const { return "!"s; }
std::string SLam::str() const { return "(lam "s + pos->str() + "."s + sbody->str() + ")"s; }
std::string SApp::str() const { return "("s + sl->str() + " "s + sr->str() + ")"s; }

/*
 * summarise
 */

UPtr<SExp> Var::summarise(VarMap& vm) const {
    auto [_, ins] = vm.emplace(name, mks<Pos>(
#if ENABLE_SMALLER_SUBTREE
        0, // tag
#endif
        nullptr, nullptr));
    assert(ins && "variable names must be unique");
    return mku<SVar>();
}

UPtr<SExp> Lam::summarise(VarMap& vm) const {
    auto sbody = body->summarise(vm);
    if (auto i = vm.find(name); i != vm.end()) {
        auto pos = i->second;
        vm.erase(i);
        return mku<SLam>(pos, std::move(sbody));
    }
    return mku<SLam>(nullptr, std::move(sbody));
}

#if ENABLE_SMALLER_SUBTREE

UPtr<SExp> App::summarise(VarMap& vml) const {
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
            lpos = mks<Pos>(rpos->tag + 1, nullptr, rpos);
        } else {
            assert(lpos->l && !lpos->r);
            lpos->r = rpos;
            lpos->tag = std::max(lpos->tag, lpos->r->tag + 1);
        }
    }

    return mku<SApp>(swap, std::move(sl), std::move(sr));
}
#else
UPtr<SExp> App::summarise(VarMap& vml) const {
    VarMap vmr;
    auto sl = l->summarise(vml);
    auto sr = r->summarise(vmr);

    for (auto& [_, lpos] : vml)
        lpos = mks<Pos>(lpos, nullptr);

    for (auto& [name, rpos] : vmr) {
        auto [i, ins] = vml.try_emplace(name, nullptr);
        auto& lpos = i->second;
        if (ins) {
            lpos = mks<Pos>(nullptr, rpos);
        } else {
            assert(lpos->l && !lpos->r);
            lpos->r = rpos;
        }
    }

    return mku<SApp>(std::move(sl), std::move(sr));
}
#endif

/*
 * rebuild
 */

UPtr<Exp> SVar::rebuild(VarMap& vm) {
    assert(vm.size() == 1);
    return mku<Var>(std::string(vm.begin()->first));
}

UPtr<Exp> SLam::rebuild(VarMap& vm) {
    static int counter = 0;
    std::string name = "x_"s + std::to_string(counter++);
    auto [_, ins] = vm.emplace(name, this->pos);
    assert(ins && "variable names must be unique");
    auto body = sbody->rebuild(vm);
    return mku<Lam>(std::move(name), std::move(body));
}

UPtr<Exp> SApp::rebuild(VarMap& vm) {
    assert(vm.size() == 1);
    return {};
}

/*
 * main
 */

int main() {
    // lam x. (y x) x
    std::cout << "exp:" << std::endl;
    auto exp = mku<Lam>("x"s, mku<App>(mku<App>(mku<Var>("y"s), mku<Var>("x"s)), mku<Var>("x"s)));
    exp->dump();

    std::cout << "summarize:" << std::endl;
    VarMap vm;
    auto sexp = exp->summarise(vm);
    sexp->dump();

    std::cout << "position of free vars:" << std::endl;
    for (auto&& [name, pos] : vm) {
        std::cout << name << ": ";
        pos->dump();
    }

    std::cout << "rebuild:" << std::endl;
    auto rexp = sexp->rebuild(vm);
    rexp->dump();
}
