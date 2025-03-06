{{ fullname | escape | underline }}

.. currentmodule:: {{ fullname }}

.. automodule:: {{ fullname }}
   :members:
   :undoc-members:
   :show-inheritance:
   :member-order: groupwise

   {% if modules %}
      .. rubric:: Modules

      .. autosummary::
          :toctree: _{{ item.name }}
          :nosignatures:
          :recursive:

      {% for item in modules %}
          {{ item.name }}
      {% endfor %}
   {% endif %}

    .. rubric:: Members
