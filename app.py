import os
import secrets
import subprocess
import time
from typing import Dict, List

from flask import Flask, redirect, render_template, request, session, url_for

app = Flask(__name__)
app.secret_key = os.environ.get("FLASK_SECRET_KEY", "change-this-secret-key")

GAME_STATES: Dict[str, Dict] = {}
ROOT_DIR = os.path.dirname(os.path.abspath(__file__))
BACKEND_EXE = os.path.join(ROOT_DIR, "mines_backend.exe")


def _get_session_id() -> str:
    sid = session.get("sid")
    if not sid:
        sid = secrets.token_hex(16)
        session["sid"] = sid
    return sid


def _state_to_stdin(state: Dict) -> str:
    lines = [
        f"{state['rows']} {state['cols']} {state['mines']} {state['game_over']} {state['remaining_safe']}"
    ]
    for cell in state["cells"]:
        lines.append(
            f"{cell['is_mine']} {cell['is_revealed']} {cell['is_flagged']} {cell['neighbor_mines']}"
        )
    return "\n".join(lines) + "\n"


def _parse_backend_output(output: str) -> Dict:
    lines = [line.strip() for line in output.splitlines() if line.strip()]
    if len(lines) < 3 or not lines[0].startswith("STATUS "):
        raise ValueError("Invalid backend output")

    status_parts = lines[0].split()
    rows = int(status_parts[1])
    cols = int(status_parts[2])
    mines = int(status_parts[3])
    remaining_safe = int(status_parts[4])
    game_over = int(status_parts[5])
    won = int(status_parts[6])
    exploded = int(status_parts[7])
    result_code = int(status_parts[8])

    if lines[1] != "BOARD":
        raise ValueError("Missing BOARD marker")

    raw_cells = lines[2:-1]
    if lines[-1] != "END" or len(raw_cells) != rows * cols:
        raise ValueError("Invalid cell payload")

    cells: List[Dict] = []
    for raw in raw_cells:
        is_mine, is_revealed, is_flagged, neighbor_mines = map(int, raw.split())
        cells.append(
            {
                "is_mine": is_mine,
                "is_revealed": is_revealed,
                "is_flagged": is_flagged,
                "neighbor_mines": neighbor_mines,
            }
        )

    return {
        "rows": rows,
        "cols": cols,
        "mines": mines,
        "remaining_safe": remaining_safe,
        "game_over": game_over,
        "won": won,
        "exploded": exploded,
        "result_code": result_code,
        "cells": cells,
    }


def _run_backend(args: List[str], stdin_data: str = "") -> Dict:
    if not os.path.exists(BACKEND_EXE):
        raise FileNotFoundError("Cannot find mines_backend.exe, compile C backend first.")

    completed = subprocess.run(
        [BACKEND_EXE] + args,
        input=stdin_data,
        text=True,
        capture_output=True,
        check=False,
    )
    if completed.returncode != 0:
        raise RuntimeError(completed.stderr.strip() or "C backend execution failed")

    return _parse_backend_output(completed.stdout)


def _new_game(rows: int, cols: int, mines: int) -> Dict:
    seed = str(int(time.time() * 1000))
    return _run_backend(["new", str(rows), str(cols), str(mines), seed])


def _count_flagged(state: Dict) -> int:
    return sum(1 for cell in state["cells"] if cell["is_flagged"])


def _prepare_view_state(state: Dict) -> Dict:
    view_state = dict(state)
    flagged = _count_flagged(state)
    view_state["flagged_count"] = flagged
    view_state["remaining_mines"] = state["mines"] - flagged
    return view_state


def _format_seconds(seconds_value: float) -> str:
    return f"{seconds_value:.2f}s"


def _update_best_score(level: str, elapsed_seconds: float) -> None:
    best_scores = session.get("best_scores", {})
    previous = best_scores.get(level)
    if previous is None or elapsed_seconds < float(previous):
        best_scores[level] = elapsed_seconds
        session["best_scores"] = best_scores


def _best_score_text(level: str) -> str:
    best_scores = session.get("best_scores", {})
    best = best_scores.get(level)
    if best is None:
        return "No record yet"
    return _format_seconds(float(best))


@app.route("/", methods=["GET"])
def index():
    sid = _get_session_id()
    if sid not in GAME_STATES:
        GAME_STATES[sid] = _new_game(9, 9, 10)
        GAME_STATES[sid]["level"] = "easy"
        GAME_STATES[sid]["started_at"] = time.time()

    state = GAME_STATES[sid]
    view_state = _prepare_view_state(state)
    elapsed_seconds = time.time() - state.get("started_at", time.time())
    level = state.get("level", "easy")
    message = ""
    if state["won"]:
        message = f"Congratulations! You won in {_format_seconds(elapsed_seconds)}."
    elif state["exploded"]:
        message = "Boom! You hit a mine."

    return render_template(
        "index.html",
        state=view_state,
        message=message,
        elapsed=_format_seconds(elapsed_seconds),
        level=level,
        best_score=_best_score_text(level),
    )


@app.route("/new", methods=["POST"])
def new_game():
    sid = _get_session_id()
    level = request.form.get("level", "medium")
    config = {
        "easy": (9, 9, 10),
        "medium": (12, 12, 24),
        "hard": (16, 16, 40),
    }
    rows, cols, mines = config.get(level, config["medium"])
    GAME_STATES[sid] = _new_game(rows, cols, mines)
    GAME_STATES[sid]["level"] = level
    GAME_STATES[sid]["started_at"] = time.time()
    return redirect(url_for("index"))


@app.route("/action", methods=["POST"])
def action():
    sid = _get_session_id()
    state = GAME_STATES.get(sid)
    if not state:
        return redirect(url_for("index"))

    if state["game_over"]:
        return redirect(url_for("index"))

    action_code = request.form.get("action", "r")
    row = int(request.form.get("row", "0"))
    col = int(request.form.get("col", "0"))
    level = state.get("level", "easy")
    started_at = state.get("started_at", time.time())

    new_state = _run_backend(
        ["act", action_code, str(row), str(col)],
        stdin_data=_state_to_stdin(state),
    )
    new_state["level"] = level
    new_state["started_at"] = started_at
    if new_state["won"]:
        elapsed_seconds = time.time() - started_at
        _update_best_score(level, elapsed_seconds)
    GAME_STATES[sid] = new_state
    return redirect(url_for("index"))


if __name__ == "__main__":
    app.run(debug=True)
