import "./styles.css";

type SortKey = "pid" | "name" | "state" | "cpuPercent" | "memoryBytes";

interface CpuMetrics {
  totalPercent: number;
}

interface MemoryMetrics {
  totalBytes: number;
  usedBytes: number;
  availableBytes: number;
  usedPercent: number;
}

interface ProcessMetrics {
  pid: number;
  name: string;
  state: string;
  cpuPercent: number;
  memoryBytes: number;
}

interface SystemSnapshot {
  timestampMs: number;
  cpu: CpuMetrics;
  memory: MemoryMetrics;
  processes: ProcessMetrics[];
}

const cpuValue = query("#cpuValue");
const cpuMeter = query("#cpuMeter");
const memoryValue = query("#memoryValue");
const memoryMeter = query("#memoryMeter");
const memoryHint = query("#memoryHint");
const processRows = query("#processRows");
const processCount = query("#processCount");
const connectionStatus = query("#connectionStatus");

let currentSnapshot: SystemSnapshot | null = null;
let sortKey: SortKey = "cpuPercent";
let sortDirection: "asc" | "desc" = "desc";

document.querySelectorAll<HTMLTableCellElement>("th[data-sort]").forEach((header) => {
  header.addEventListener("click", () => {
    const nextSortKey = header.dataset.sort as SortKey;
    if (sortKey === nextSortKey) {
      sortDirection = sortDirection === "asc" ? "desc" : "asc";
    } else {
      sortKey = nextSortKey;
      sortDirection = sortKey === "name" || sortKey === "state" ? "asc" : "desc";
    }

    render();
  });
});

void loadSnapshot();
connectEvents();

function query(selector: string): HTMLElement {
  const element = document.querySelector<HTMLElement>(selector);
  if (!element)
    throw new Error(`Missing element: ${selector}`);

  return element;
}

async function loadSnapshot() {
  const response = await fetch("/api/snapshot");
  updateSnapshot(await response.json() as SystemSnapshot);
}

function connectEvents() {
  const events = new EventSource("/api/events");

  events.onopen = () => {
    connectionStatus.textContent = "live";
    connectionStatus.classList.remove("stale");
  };

  events.onmessage = (event) => {
    updateSnapshot(JSON.parse(event.data) as SystemSnapshot);
  };

  events.onerror = () => {
    connectionStatus.textContent = "reconnecting";
    connectionStatus.classList.add("stale");
  };
}

function updateSnapshot(snapshot: SystemSnapshot) {
  currentSnapshot = snapshot;
  render();
}

function render() {
  if (!currentSnapshot)
    return;

  const cpuPercent = clampPercent(currentSnapshot.cpu.totalPercent);
  const memoryPercent = clampPercent(currentSnapshot.memory.usedPercent);

  cpuValue.textContent = `${formatPercent(cpuPercent)}%`;
  cpuMeter.style.width = `${cpuPercent}%`;
  memoryValue.textContent = `${formatPercent(memoryPercent)}%`;
  memoryMeter.style.width = `${memoryPercent}%`;
  memoryHint.textContent = `${formatBytes(currentSnapshot.memory.usedBytes)} / ${formatBytes(currentSnapshot.memory.totalBytes)}`;

  const processes = [...currentSnapshot.processes].sort(compareProcesses);
  processCount.textContent = String(processes.length);
  processRows.replaceChildren(...processes.map(renderProcess));
}

function renderProcess(process: ProcessMetrics) {
  const row = document.createElement("tr");
  row.append(
    cell(String(process.pid), "mono"),
    cell(process.name),
    cell(process.state, "state"),
    cell(`${formatPercent(process.cpuPercent)}%`, "number"),
    cell(formatBytes(process.memoryBytes), "number"),
  );

  return row;
}

function cell(text: string, className?: string) {
  const element = document.createElement("td");
  element.textContent = text;
  if (className)
    element.className = className;

  return element;
}

function compareProcesses(left: ProcessMetrics, right: ProcessMetrics) {
  const direction = sortDirection === "asc" ? 1 : -1;
  const leftValue = left[sortKey];
  const rightValue = right[sortKey];

  if (typeof leftValue === "string" && typeof rightValue === "string")
    return leftValue.localeCompare(rightValue) * direction;

  if (leftValue < rightValue)
    return -1 * direction;
  if (leftValue > rightValue)
    return 1 * direction;

  return left.pid - right.pid;
}

function clampPercent(value: number) {
  return Math.max(0, Math.min(100, value));
}

function formatPercent(value: number) {
  return value.toFixed(1);
}

function formatBytes(value: number) {
  const units = ["B", "KiB", "MiB", "GiB", "TiB"];
  let amount = value;
  let unitIndex = 0;

  while (amount >= 1024 && unitIndex < units.length - 1) {
    amount /= 1024;
    unitIndex += 1;
  }

  return `${amount.toFixed(unitIndex === 0 ? 0 : 1)} ${units[unitIndex]}`;
}
