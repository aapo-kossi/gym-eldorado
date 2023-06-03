from eldorado_environment.env.environment import raw_eldoradoenv

# Hide pygame support prompt
import os
os.environ['PYGAME_HIDE_SUPPORT_PROMPT'] = 'hide'


from gymnasium.envs.registration import register

register(
    id = "eldorado-v1",
    entry_point = "eldorado-environment.env:raw_eldoradoenv",
)
