#include <array>
#include <iostream>
#include <random>

const int NUM_ACTIONS = 3;

struct Agent {
    std::array<double, 3> regretSum = {0, 0, 0};
    std::array<double, 3> strategySum = {0, 0, 0};
};

auto getActionUtility(int oppAction) -> std::array<int, 3> {
    std::array<int, 3> actionUtility;
    actionUtility[oppAction] = 0;
    actionUtility[oppAction == NUM_ACTIONS - 1 ? 0 : oppAction + 1] = 1;
    actionUtility[oppAction == 0 ? NUM_ACTIONS - 1 : oppAction - 1] = -1;
    return actionUtility;
}

auto getStrategy(Agent& agent) -> std::array<double, 3> {
    double normalizingSum = 0;
    std::array<double, 3> strategy;
    for (int i=0;i<NUM_ACTIONS;i++) {
        strategy[i] = agent.regretSum[i] > 0 ? agent.regretSum[i]:0;
        normalizingSum += strategy[i];
    }

    for (int i=0;i<NUM_ACTIONS;i++) {
        if (normalizingSum > 0) {
            strategy[i] /= normalizingSum;
        } else {
            strategy[i] = 1.0 / NUM_ACTIONS;
        }
        agent.strategySum[i] += strategy[i];
    }

    return strategy;
}

auto getAction(const std::array<double, 3>& strategy) -> int {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<double> distr(0.0, 1.0);
    double selector = distr(gen);
    if (selector < strategy[0]) {
        return 0;
    } else if (selector < (strategy[0]+strategy[1])) {
        return 1;
    } else {
        return 2;
    }
}

auto getAverageStrategy(const Agent& agent) -> std::array<double, 3> {
    std::array<double, 3> avgStrategy;
    double normalizingSum = 0;
    for (int i=0;i<NUM_ACTIONS;i++) {
        normalizingSum += agent.strategySum[i];
    }
    for (int i=0;i<NUM_ACTIONS;i++) {
        if (normalizingSum > 0) {
            avgStrategy[i] = agent.strategySum[i] / normalizingSum;
        } else {
            avgStrategy[i] = 1.0 / NUM_ACTIONS;
        }
    }
    return avgStrategy;
}

auto trainSelfPlay(int iterations) -> void {
    Agent p1, p2;
    for (int i=0;i<iterations;++i) {
        auto curStrategy1 = getStrategy(p1);
        auto curStrategy2 = getStrategy(p2);

        auto action1 = getAction(curStrategy1);
        auto action2 = getAction(curStrategy2);

        auto actionUtility1 = getActionUtility(action2);
        auto actionUtility2 = getActionUtility(action1);

        for (int j=0;j<NUM_ACTIONS;j++) {
            p1.regretSum[j] += actionUtility1[j] - actionUtility1[action1];
            p2.regretSum[j] += actionUtility2[j] - actionUtility2[action2];
        }
    }
    auto finalStrategy1 = getAverageStrategy(p1);
    std::cout << "Self-Play GTO Strategy: " << finalStrategy1[0] << ' ' << finalStrategy1[1] << ' ' << finalStrategy1[2] << std::endl;
}

auto trainAgainstFixed(int iterations, std::array<double, 3> oppStrategy) -> void {
    Agent p1;
    for (int i=0;i<iterations;++i) {
        auto curStrategy1 = getStrategy(p1);

        auto action1 = getAction(curStrategy1);
        auto action2 = getAction(oppStrategy);

        auto actionUtility1 = getActionUtility(action2);

        for (int j=0;j<NUM_ACTIONS;j++) {
            p1.regretSum[j] += actionUtility1[j] - actionUtility1[action1];
        }
    }
    auto finalStrategy1 = getAverageStrategy(p1);
    std::cout << "Best Response vs Fixed [" << oppStrategy[0] << ", " << oppStrategy[1] << ", " << oppStrategy[2] << "]: "
              << finalStrategy1[0] << ' ' << finalStrategy1[1] << ' ' << finalStrategy1[2] << std::endl;
}

int main() {
    std::cout << "--- Training Self-Play (Nash Equilibrium) ---" << std::endl;
    trainSelfPlay(1e7);

    std::cout << "\n--- Training Against Fixed Opponent (Best Response) ---" << std::endl;
    trainAgainstFixed(1e7, {0.4, 0.3, 0.3});

    return 0;
}
