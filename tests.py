from eldorado_environment import eldorado_env, flat_eldorado_env
from pettingzoo.test import api_test
import logging

def test_iteration():
    env = eldorado_env()
    for _ in range(10):
        env.reset()
        for i in range(1000):
            env.step(env.action_space(env.agent_selection).sample(env.observe(env.agent_selection)['action_mask']))
            if any(env.truncations.values()) or any(env.terminations.values()):
                break

def test_pettingzoo():
    env = flat_eldorado_env()
    api_test(env)