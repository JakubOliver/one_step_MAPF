#!/usr/bin/env python3

import argparse
import json
import re
import shutil
from pathlib import Path


MAPS = (
    "Berlin_1_256",
    "empty-32-32",
    "warehouse-10-20-10-2-1",
    "room-64-64-16",
    "random-32-32-10",
)

SCENARIO_TYPES = (
    "even",
    "random",
)


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Select MovingAI cases for the benchmark matrix.",
    )
    parser.add_argument(
        "--scenario-count",
        type=int,
        required=True,
        help="number of scenarios to select for each map and scenario type",
    )
    parser.add_argument(
        "--extracted-dir",
        type=Path,
        default=Path("extracted"),
        help="directory containing the extracted MovingAI archives",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path("benchmark-data"),
        help="directory where selected maps and scenarios are copied",
    )
    parser.add_argument(
        "--github-output",
        type=Path,
        required=True,
        help="GitHub Actions output file",
    )

    return parser.parse_args()


def scenario_index(path: Path) -> int:
    match = re.search(r"-(\d+)\.scen$", path.name)

    if match is None:
        return 10**9

    return int(match.group(1))


def find_single_map(extracted_dir: Path, map_name: str) -> Path:
    matches = list(extracted_dir.rglob(f"{map_name}.map"))

    if len(matches) != 1:
        raise RuntimeError(
            f"Expected exactly one map for {map_name}, found {len(matches)}"
        )

    return matches[0]


def select_cases(
    scenario_count: int,
    extracted_dir: Path,
    output_dir: Path,
) -> list[dict[str, str]]:
    maps_dir = output_dir / "maps"
    scenarios_dir = output_dir / "scenarios"
    maps_dir.mkdir(parents=True, exist_ok=True)
    scenarios_dir.mkdir(parents=True, exist_ok=True)

    cases = []

    for map_name in MAPS:
        source_map = find_single_map(extracted_dir, map_name)
        target_map = maps_dir / source_map.name
        shutil.copy2(source_map, target_map)

        for scenario_type in SCENARIO_TYPES:
            matches = sorted(
                extracted_dir.rglob(f"{map_name}-{scenario_type}-*.scen"),
                key=scenario_index,
            )
            selected = matches[:scenario_count]

            if len(selected) < scenario_count:
                raise RuntimeError(
                    f"Requested {scenario_count} scenarios for "
                    f"{map_name}/{scenario_type}, but only found {len(selected)}"
                )

            for scenario in selected:
                target_scenario = scenarios_dir / scenario.name
                shutil.copy2(scenario, target_scenario)

                cases.append(
                    {
                        "map": map_name,
                        "map_file": str(Path("maps") / target_map.name),
                        "scenario_type": scenario_type,
                        "scenario": scenario.stem,
                        "scenario_file": str(
                            Path("scenarios") / target_scenario.name
                        ),
                    }
                )

    return cases


def write_github_output(path: Path, cases: list[dict[str, str]]) -> None:
    with path.open("a", encoding="utf-8") as github_output:
        github_output.write(
            "cases=" + json.dumps(cases, separators=(",", ":")) + "\n"
        )


def main() -> None:
    arguments = parse_arguments()
    cases = select_cases(
        arguments.scenario_count,
        arguments.extracted_dir,
        arguments.output_dir,
    )

    print(json.dumps(cases, indent=2))
    write_github_output(arguments.github_output, cases)


if __name__ == "__main__":
    main()
