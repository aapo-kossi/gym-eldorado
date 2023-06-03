from eldorado_environment.env.environment import raw_eldoradoenv, eldorado_env

# Hide pygame support prompt
import os
os.environ['PYGAME_HIDE_SUPPORT_PROMPT'] = 'hide'


# from gymnasium.envs.registration import register

# register(
#     id = "eldorado-v0",
#     entry_point = "eldorado_environment.env:eldorado_env",
# )
