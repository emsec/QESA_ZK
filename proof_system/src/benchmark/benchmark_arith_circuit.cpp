#include "test.h"
#include "qesa/qesa_zk.h"
#include "timer.h"

using namespace qesa;

bool benchmark_arith_circuit()
{
    // std::string circuit = "../circuits/eqtest.arith";
    std::string circuit = "../big_circuits/sha.arith";

    std::vector<SparseMatrix> matrices;
    u32 num_variables = 0;

    std::cout << "reading matrices..." << std::flush;

    {
        std::ifstream infile(circuit + "_mat");
        std::string line;
        if (std::getline(infile, line))
        {
            SparseMatrix current_matrix;
            num_variables = std::stoi(line.substr(0, line.find(" ")));
            while (std::getline(infile, line))
            {
                auto comment_start = line.find('#');
                if (comment_start != std::string::npos)
                {
                    line = rtrim(line.substr(0, comment_start));
                }
                if (line.empty())
                {
                    if (!current_matrix.vectors.empty())
                    {
                        matrices.push_back(current_matrix);
                    }
                    current_matrix.vectors.clear();
                    continue;
                }
                auto vectors = split(line, ';');

                math::SparseVector<BN> svec_a;
                auto a_parts = split(vectors[0], ',');
                for (const auto &element : a_parts)
                {
                    auto assignment = split(element, ':');
                    svec_a.set(std::stoi(assignment[0]), BN(assignment[1]));
                }

                math::SparseVector<BN> svec_b;
                auto b_parts = split(vectors[1], ',');
                for (const auto &element : b_parts)
                {
                    auto assignment = split(element, ':');
                    svec_b.set(std::stoi(assignment[0]), BN(assignment[1]));
                }

                current_matrix.vectors.emplace_back(svec_a, svec_b);
            }
            if (!current_matrix.vectors.empty())
            {
                matrices.push_back(current_matrix);
            }
        }
    }
    if (num_variables == 0)
    {
        std::cout << "file '" << circuit << "' is not a valid matrix data file" << std::endl;
        return false;
    }

    std::cout << "OK!" << std::endl;

    std::cout << "reading witness..." << std::flush;
    math::vector<BN> witness;
    witness.reserve(num_variables);
    {
        std::ifstream infile(circuit + "_wit");
        std::string line;
        while (std::getline(infile, line, ','))
        {
            witness.emplace_back(rtrim(line));
        }
    }

    std::cout << "OK!" << std::endl
              << std::endl;
    //std::cout << "witness zero: " << n - witness.non_zero_elements() << "/" << n << std::endl;
    std::cout << "#wires = " << num_variables << std::endl
              << std::endl;

    {
        std::cout << "performing sanity check..." << std::flush;
        u32 cnt = 0;
        for (const auto &mat : matrices)
        {
            cnt++;
            BN eval = 0;
            for (const auto& [a, b] : mat.vectors)
            {
                eval += ((a * witness) * (b * witness)) % G::order();
            }
            eval %= G::order();
            if (eval != 0)
            {
                std::cout << "ERROR at matrix " << cnt << std::endl;
                std::cout << mat << std::endl;
                for (const auto& [a, b] : mat.vectors)
                {
                    for (auto it = a.begin(); it != a.end(); ++it)
                    {
                        std::cout << "_" << it.index() << "_ = " << witness[it.index()] << "_16" << std::endl;
                    }
                    for (auto it = b.begin(); it != b.end(); ++it)
                    {
                        std::cout << "_" << it.index() << "_ = " << witness[it.index()] << "_16" << std::endl;
                    }
                    std::cout << std::endl;
                }
                std::cout << "0 != " << eval << std::endl;
                return false;
            }
        }
        std::cout << "OK!" << std::endl
                  << std::endl;
    }

    try
    {
        u32 timings[2] = {0, 0};

        CRS crs = gen_CRS(num_variables);

        qesa_zk::ProverContext pctx(crs);
        qesa_zk::VerifierContext vctx(crs);

        timer::start();
        qesa_zk::begin(pctx, matrices, witness);
        timer::stop();
        timings[0] += timer::milliseconds();

        timer::start();
        qesa_zk::begin(vctx, matrices);
        timer::stop();
        timings[1] += timer::milliseconds();

        std::vector<u8> buffer;
        bool continue_prover = true;
        bool continue_verifier = true;
        while (continue_prover || continue_verifier)
        {
            if (continue_prover)
            {
                timer::start();
                continue_prover = qesa_zk::step_prover(pctx, buffer);
                timer::stop();
                timings[0] += timer::milliseconds();
            }
            if (continue_verifier)
            {
                timer::start();
                continue_verifier = qesa_zk::step_verifier(vctx, buffer);
                timer::stop();
                timings[1] += timer::milliseconds();
            }
        }
        timer::start();
        auto result = qesa_zk::get_result(vctx);
        timer::stop();
        timings[1] += timer::milliseconds();

        std::cout << "proof verification: ";
        if (result)
        {
            std::cout << "OK!" << std::endl;
        }
        else
        {
            std::cout << "ERROR!" << std::endl;
        }
        std::cout << std::endl;
        std::cout << "Prover: " << std::setw(4) << timings[0] << "ms, Verifier: " << std::setw(4) << timings[1] << "ms, Total: " << std::setw(4) << (timings[0] + timings[1]) << "ms" << std::endl;
    }
    catch (std::runtime_error &ex)
    {
        std::cout << ex.what() << std::endl;
    }

    return true;
}
