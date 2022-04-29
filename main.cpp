#include <iostream>
#include <memory>
#include <optional>
#include <string_view>
#include <unordered_set>

#include "hash.h"

/*
 * Exp
 */

struct Exp {
    virtual ~Exp() {}
};

struct Var : public Exp {
    Var(std::string_view name)
        : name(name) {}

    std::string name;
};

struct Lam : public Exp {
    Lam(std::string_view name, const Exp* body)
        : name(name)
        , body(body) {}

    std::string name;
    std::unique_ptr<const Exp> body;
};

struct App : public Exp {
    App(const Exp* callee, const Exp* arg)
        : callee(callee)
        , arg(arg) {}

    std::unique_ptr<const Exp> callee;
    std::unique_ptr<const Exp> arg;
};

/*
 * PTree (PosTree in the paper)
 */

struct PTree {
    PTree() = default;
    PTree(std::unique_ptr<const PTree>&& l, std::unique_ptr<const PTree>&& r)
        : l(std::move(l))
        , r(std::move(r)) {}

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
 * SExp (Structure in the paper)
 */

enum Tag : uint8_t { SVar, SLam, SApp };

struct SExp {
    SExp()
        : id_(id_counter_++) {}
    virtual ~SExp() {}

    hash_t hash() const { return hash_; }

protected:
    static hash_t id_counter_;
    hash_t id_;
    hash_t hash_;
};

size_t SExp::id_counter_ = 0;

struct SVar : public SExp {
    SVar() {
        hash_ = hash_begin(uint8_t(Tag::SVar));
    }
};

struct SLam : public SExp {
    SLam(std::unique_ptr<const PTree>&& ptree, const SExp* body)
        : ptree(std::move(ptree))
        , body(body) {
        hash_ = hash_begin(uint8_t(Tag::SLam));
    }

    std::unique_ptr<const PTree> ptree;
    const SExp* body;
};

struct SApp : public SExp {
    SApp(const SExp* callee, const SExp* arg)
        : callee(callee)
        , arg(arg) {
        hash_ = hash_begin(uint8_t(Tag::SApp));
    }

    const SExp* callee;
    const SExp* arg;
};

struct SExpHash {
    hash_t operator()(const SExp* e) const { return e->hash(); }
};

struct SExpEq {
    bool operator()(const SExp* e1, const SExp* e2) const { return e1 == e2; }
};

struct World {
    std::unordered_set<const SExp*, SExpHash, SExpEq> set;
};

int main() {
    auto ptree = pt_both(pt_r(pt_here()), pt_here());
    return EXIT_SUCCESS;
}
