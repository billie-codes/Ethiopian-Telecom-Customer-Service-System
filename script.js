/* Ethiopian Telecom Customer Service System - browser version
   Hierarchy: Country -> Region -> City -> Branch -> Customer
   Connected to index.html (form ids) and style.css (classes). */

const country = { regions: [] };

const $ = (id) => document.getElementById(id);
const esc = (s) => String(s).replace(/[&<>"']/g, (c) => ({ "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;", "'": "&#39;" }[c]));
const fmt = (n, d = 2) => Number(n).toLocaleString("en-US", { minimumFractionDigits: d, maximumFractionDigits: d });

/* ---------- Input validation ---------- */
function readNumber(id, label, integer = false) {
  const raw = $(id).value.trim();
  if (raw === "") throw new Error(label + " is required.");
  const n = Number(raw);
  if (!Number.isFinite(n)) throw new Error(label + " must be a number.");
  if (n < 0) throw new Error(label + " cannot be negative.");
  if (integer && !Number.isInteger(n)) throw new Error(label + " must be a whole number.");
  return n;
}
function readText(id, label) {
  const v = $(id).value.trim();
  if (!v) throw new Error(label + " is required.");
  return v;
}

/* ---------- Data entry ---------- */
function findOrAdd(list, name, make) {
  let item = list.find((x) => x.name.toLowerCase() === name.toLowerCase());
  if (!item) { item = make(name); list.push(item); }
  return item;
}

function addCustomer(e) {
  e.preventDefault();
  const msg = $("msg");
  try {
    const region = readText("region", "Region");
    const city = readText("city", "City");
    const branch = readText("branch", "Branch");
    const customer = {
      id: readText("cid", "Customer name/ID"),
      balance: readNumber("balance", "Mobile balance"),
      data: readNumber("data", "Data usage"),
      calls: readNumber("calls", "Call minutes"),
      sms: readNumber("sms", "SMS count", true),
    };
    const r = findOrAdd(country.regions, region, (n) => ({ name: n, cities: [] }));
    const c = findOrAdd(r.cities, city, (n) => ({ name: n, branches: [] }));
    const b = findOrAdd(c.branches, branch, (n) => ({ name: n, customers: [] }));
    b.customers.push(customer);
    setMsg("Added " + customer.id + " to " + branch + ".", true);
    ["cid", "balance", "data", "calls", "sms"].forEach((id) => ($(id).value = ""));
    $("cid").focus();
    updateStatus();
  } catch (err) {
    setMsg(err.message, false);
  }
}

function setMsg(text, ok) {
  const m = $("msg");
  m.textContent = text;
  m.className = "msg " + (ok ? "ok" : "bad");
}

function updateStatus() {
  const t = totals(country);
  $("status").textContent = t.regions + " region(s), " + t.branches + " branch(es), " + t.count + " customer(s) entered.";
}

/* ---------- Recursive computation ---------- */
const kids = (n) => n.regions || n.cities || n.branches || null;

// Recursively totals customers, balance, data, calls, SMS and branches.
function totals(node) {
  const sub = kids(node);
  if (!sub) { // branch: base case
    const t = { regions: 0, branches: 1, count: node.customers.length, balance: 0, data: 0, calls: 0, sms: 0 };
    node.customers.forEach((c) => { t.balance += c.balance; t.data += c.data; t.calls += c.calls; t.sms += c.sms; });
    return t;
  }
  const t = { regions: node.regions ? sub.length : 0, branches: 0, count: 0, balance: 0, data: 0, calls: 0, sms: 0 };
  sub.forEach((child) => {
    const s = totals(child);
    ["branches", "count", "balance", "data", "calls", "sms"].forEach((k) => (t[k] += s[k]));
  });
  return t;
}

const usageScore = (t) => t.data + t.calls / 60; // same rule as the C++ version
const divide = (a, b) => (b === 0 ? 0 : a / b); // division-by-zero guard

// Returns the item of a list with the highest value of fn(item).
function best(list, fn) {
  return list.reduce((top, x) => (top === null || fn(x) > fn(top) ? x : top), null);
}

/* ---------- Report ---------- */
function statCard(label, value) {
  return '<div class="stat"><span>' + esc(label) + "</span><strong>" + esc(value) + "</strong></div>";
}

function branchHTML(b) {
  const t = totals(b);
  const top = best(b.customers, (c) => c.balance);
  const rows = b.customers.map((c) =>
    "<tr><td>" + esc(c.id) + "</td><td>" + fmt(c.balance) + "</td><td>" + fmt(c.data, 3) + "</td><td>" + fmt(c.calls, 1) + "</td><td>" + c.sms + "</td></tr>").join("");
  return '<details class="lvl branch"><summary>Branch: ' + esc(b.name) + " <em>" + t.count + " customers</em></summary>" +
    "<p>Data " + fmt(t.data, 3) + " GB · Calls " + fmt(t.calls, 1) + " min · SMS " + t.sms +
    "<br>Highest balance: <b>" + esc(top.id) + "</b> (" + fmt(top.balance) + " ETB)</p>" +
    '<div class="scroll"><table><tr><th>Customer</th><th>Balance (ETB)</th><th>Data (GB)</th><th>Calls (min)</th><th>SMS</th></tr>' + rows + "</table></div></details>";
}

function cityHTML(c) {
  const t = totals(c);
  const topB = best(c.branches, (b) => b.customers.length);
  return '<details class="lvl city"><summary>City: ' + esc(c.name) + " <em>" + t.count + " customers</em></summary>" +
    "<p>Data " + fmt(t.data, 3) + " GB · Calls " + fmt(t.calls, 1) + " min · SMS " + t.sms +
    "<br>Largest branch: <b>" + esc(topB.name) + "</b> (" + topB.customers.length + " customers)</p>" +
    c.branches.map(branchHTML).join("") + "</details>";
}

function regionHTML(r) {
  const t = totals(r);
  const topC = best(r.cities, (c) => usageScore(totals(c)));
  return '<details class="lvl region" open><summary>Region: ' + esc(r.name) + " <em>" + t.count + " customers</em></summary>" +
    "<p>Data " + fmt(t.data, 3) + " GB · Calls " + fmt(t.calls, 1) + " min · SMS " + t.sms +
    "<br>Highest-usage city: <b>" + esc(topC.name) + "</b></p>" +
    r.cities.map(cityHTML).join("") + "</details>";
}

function generateReport() {
  const out = $("report");
  if (country.regions.length === 0) {
    out.innerHTML = '<p class="msg bad">Add at least one customer first.</p>';
    return;
  }
  const t = totals(country);
  const topR = best(country.regions, (r) => totals(r).count);
  out.innerHTML =
    "<h3>National statistics</h3>" +
    '<div class="stats">' +
    statCard("Regions", t.regions) + statCard("Branches", t.branches) + statCard("Customers", t.count) +
    statCard("Total data (GB)", fmt(t.data, 3)) + statCard("Total calls (min)", fmt(t.calls, 1)) + statCard("Total SMS", t.sms) +
    statCard("Avg balance (ETB)", fmt(divide(t.balance, t.count))) + statCard("Avg data (GB)", fmt(divide(t.data, t.count), 3)) +
    statCard("Avg calls (min)", fmt(divide(t.calls, t.count), 1)) +
    "</div>" +
    "<p>Region with most customers: <b>" + esc(topR.name) + "</b> (" + totals(topR).count + ")</p>" +
    "<h3>Hierarchical report</h3>" + country.regions.map(regionHTML).join("");
}

/* ---------- Sample data and reset ---------- */
function loadSample() {
  country.regions = [
    { name: "Oromia", cities: [{ name: "Adama", branches: [{ name: "Adama Branch 1", customers: [
      { id: "Abe", balance: 500, data: 10.5, calls: 120, sms: 30 },
      { id: "Bekele", balance: 200, data: 5, calls: 60, sms: 10 }] }] },
      { name: "Dire Dawa Road", branches: [{ name: "DD Branch 1", customers: [
      { id: "Hana", balance: 150, data: 2, calls: 40, sms: 5 }] }] }] },
    { name: "Amhara", cities: [{ name: "Bahir Dar", branches: [{ name: "BD Branch 1", customers: [
      { id: "Chala", balance: 350, data: 8, calls: 90, sms: 20 }] }] }] },
  ];
  setMsg("Sample data loaded.", true);
  updateStatus();
  generateReport();
}

function resetAll() {
  country.regions = [];
  $("report").innerHTML = "";
  setMsg("All data cleared.", true);
  updateStatus();
}

/* ---------- Wire up the page ---------- */
document.addEventListener("DOMContentLoaded", () => {
  $("entry-form").addEventListener("submit", addCustomer);
  $("btn-report").addEventListener("click", generateReport);
  $("btn-sample").addEventListener("click", loadSample);
  $("btn-reset").addEventListener("click", resetAll);
  updateStatus();
});
