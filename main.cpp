#include <iostream>
#include <memory>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include "hash.h"

struct SExp;
struct World;

/*
 * PTree (PosTree in the paper)
 */

struct PTree {
    PTree() = default;
    PTree(std::unique_ptr<const PTree>&& l, std::unique_ptr<const PTree>&& r)
        : l(std::move(l))
        , r(std::move(r)) {}
    PTree(PTree&& other) {
        swap(*this, other);
    }

    friend void swap(PTree& p1, PTree& p2) {
        using std::swap;
        swap(p1.l, p2.l);
        swap(p1.r, p2.r);
    }

    bool here() const { return !l && !r; }
    std::unique_ptr<const PTree> l;
    std::unique_ptr<const PTree> r;
};

using PTreePtr = std::unique_ptr<const PTree>;

PTreePtr pt_here() { return std::make_unique<const PTree>(); }
PTreePtr pt_l(PTreePtr&& l) { return std::make_unique<const PTree>(std::move(l), nullptr); }
PTreePtr pt_r(PTreePtr&& r) { return std::make_unique<const PTree>(nullptr, std::move(r)); }
PTreePtr pt_both(PTreePtr&& l, PTreePtr&& r) { return std::make_unique<const PTree>(std::move(l), std::move(r)); }

/*
 * VarMap
 */

using VarMap = std::unordered_map<std::string, PTreePtr>;

/*
 * Exp
 */

struct ESum {
    ESum(const SExp* sexp, VarMap&& vars)
        : sexp(sexp)
        , vars(std::move(vars)) {}

    const SExp* sexp;
    VarMap vars;
};

struct Exp {
    virtual ~Exp() {}
    virtual ESum summarize(World&) const;
};

struct Var : public Exp {
    Var(std::string_view name)
        : name(name) {}

    ESum summarize(World& world) const override;

    std::string name;
};

struct Lam : public Exp {
    Lam(std::string_view name, const Exp* body)
        : name(name)
        , body(body) {}

    ESum summarize(World& world) const override;

    std::string name;
    std::unique_ptr<const Exp> body;
};

struct App : public Exp {
    App(const Exp* callee, const Exp* arg)
        : callee(callee)
        , arg(arg) {}

    ESum summarize(World& world) const override;

    std::unique_ptr<const Exp> callee;
    std::unique_ptr<const Exp> arg;
};

/*
 * SExp (Structure in the paper)
 */

enum Tag : uint8_t { SVar, SLam, SApp };

struct SExp {
    SExp(World& world)
        : world(world)
        , id_(id_counter_++) {}
    virtual ~SExp() {}

    hash_t hash() const { return hash_; }

    World& world;

protected:
    static hash_t id_counter_;
    hash_t id_;
    hash_t hash_;
};

size_t SExp::id_counter_ = 0;

struct SVar : public SExp {
    SVar(World& world)
        : SExp(world) {
        hash_ = hash_begin(uint8_t(Tag::SVar));
    }
};

struct SLam : public SExp {
    SLam(World& world, std::optional<PTreePtr>&& ptree, const SExp* body)
        : SExp(world)
        , ptree(std::move(ptree))
        , body(body) {
        hash_ = hash_begin(uint8_t(Tag::SLam));
    }

    std::optional<PTreePtr> ptree;
    const SExp* body;
};

struct SApp : public SExp {
    SApp(World& world, const SExp* callee, const SExp* arg)
        : SExp(world)
        , callee(callee)
        , arg(arg) {
        hash_ = hash_begin(uint8_t(Tag::SApp));
    }

    const SExp* callee;
    const SExp* arg;
};

/*
 * World/Hashing
 */

struct SExpHash {
    hash_t operator()(const SExp* e) const { return e->hash(); }
};

struct SExpEq {
    bool operator()(const SExp* e1, const SExp* e2) const { return e1 == e2; }
};

struct World {
    const SExp* svar();
    const SExp* slam(PTreePtr&& ptree, const SExp* body);
    const SExp* sapp(const SExp* callee, const Exp* arg);

    //const SExp* insert(

    std::unordered_set<const SExp*, SExpHash, SExpEq> set;
};

/*
 * summarize
 */

ESum Var::summarize(World& world) const {
    VarMap vm;
    vm.emplace(name, pt_here());
    return {world.svar(), std::move(vm)};
}

ESum Lam::summarize(World& world) const {
    auto&& [sbody, vm] = body->summarize(world);
    if (auto i = vm.find(name); i != vm.end()) {
        auto&& ptree = std::move(i->second);
        vm.erase(i);
        return {world.slam(std::move(ptree), sbody), std::move(vm)};
    }
    return {world.slam({}, sbody), std::move(vm)};
}

int main() {
    auto ptree = pt_both(pt_r(pt_here()), pt_here());
    return EXIT_SUCCESS;
}
