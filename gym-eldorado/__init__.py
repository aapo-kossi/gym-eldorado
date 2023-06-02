# Hide pygame support prompt
import os
os.environ['PYGAME_HIDE_SUPPORT_PROMPT'] = '1'


from gymnasium.envs.registration import register

register(
    id = "eldorado-v1",
    entry_point = "gym-eldorado.env:EldoradoVenv",
)