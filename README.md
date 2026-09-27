# esp-components

Custom ESP32 component collection used for ESP firmware projects.

## Repository skeleton

```text
components/
  sample_component/
    CMakeLists.txt
    idf_component.yml
    include/
      sample_component.h
    src/
      sample_component.c
    examples/
      basic/
        CMakeLists.txt
        main/
          CMakeLists.txt
          main.c
        sdkconfig.defaults
```

## Workflow for adding a new component

1. Create a new folder under `components/<component_name>/`.
2. Add the component build/config files:
   - `CMakeLists.txt`
   - `idf_component.yml`
3. Add public headers in `include/` and implementation files in `src/`.
4. Add at least one runnable example under `components/<component_name>/examples/<example_name>/`.
5. Keep each example as a standalone ESP-IDF project with:
   - top-level `CMakeLists.txt`
   - `main/CMakeLists.txt`
   - `main/main.c`
   - optional `sdkconfig.defaults`
6. Validate the example builds in ESP-IDF before opening a PR.
