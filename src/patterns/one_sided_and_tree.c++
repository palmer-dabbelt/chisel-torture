/*
 * Copyright (C) 2014 Palmer Dabbelt
 *   <palmer.dabbelt@eecs.berkeley.edu>
 *
 * This file is part of chisel-torture.
 *
 * chisel-torture is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * chisel-torture is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with flo-llvm.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <libcgraph/pattern.h++>
#include <libcgraph/pattern_factory.h++>
#include <libcgraph/pattern_store.h++>
#include <libcgraph/reg.h++>
#include <gmpxx.h>

class one_sided_and_tree: public libcgraph::pattern {
private:
    std::shared_ptr<libcgraph::node> out;
    ssize_t cycle;

public:
    one_sided_and_tree(size_t _width, size_t _depth)
        : cycle(0)
        {
            auto in = std::make_shared<libcgraph::node>();
            _inputs.push_back(in);
            auto op = std::make_shared<libcgraph::operation>(
                std::make_shared<libcgraph::node>(),
                _width,
                libflo::opcode::AND,
                std::vector<std::shared_ptr<libcgraph::node>>({in, in})
                );
            _compute.push_back(op);

            for (size_t i = 0; i < _depth; ++i) {
                auto n = std::make_shared<libcgraph::node>();
                op = op & n;
                _compute.push_back(op);
                _inputs.push_back(n);
            }

            out = op->d();
            _outputs = {out};
        }

    void step(void)
        {
            mpz_class cur = _inputs[0]->value();
            for (const auto& i: _inputs)
                cur &= i->value();

            out->update(cur, cycle);
            cycle++;
        }
};

class one_sided_and_tree_factory: public libcgraph::pattern_factory {
    typedef std::shared_ptr<libcgraph::pattern> pattern_ptr;

    const std::string name(void) const
        {
            return "one_sided_and_tree";
        }

    const pattern_ptr create(void) const
        {
            return std::make_shared<one_sided_and_tree>(32, 128);
        }

    const pattern_ptr create(const std::string& args) const
        {
            int width, depth;
            if (sscanf(args.c_str(), "%d-%d", &width, &depth) != 2) {
                fprintf(stderr, "Unable to parse args '%s'\n", args.c_str());
                abort();
            }

            return std::make_shared<one_sided_and_tree>(width, depth);
        }

    std::vector<std::string> examples(void) const
        {
            return {"64-4", "64-8", "32-128", "64-128"};
        }
};

static void cons(void) __attribute__((constructor));
void cons(void)
{
    auto factory = std::make_shared<one_sided_and_tree_factory>();
    libcgraph::pattern_store::link(factory);
}
