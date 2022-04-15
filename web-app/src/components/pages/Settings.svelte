<script lang="ts">
  import { onMount } from "svelte";
  import api from "../../api";
  import Container from "../templates/Container.svelte";
  import Snackbar from "../atoms/Snackbar.svelte";

  async function initSettings() {
    const config = await api.config();
    now = new Date(config.time);
    refresh = config.refresh;
    order = config.shuffle ? "shuffle" : "normal";
  }

  function applySettings() {
    api
      .config({ time: now.getTime(), refresh, shuffle: order == "shuffle" })
      .then((res) => {
        snackbar.text = `Update settings ${res.ok ? "succeeded" : "failed"}`;
        snackbar.error = !res.ok;
        snackbar.show = true;
      });
  }

  onMount(initSettings);

  let snackbar = {
    show: false,
    text: "",
    error: false,
  };

  let now: Date = new Date();
  $: shift = new Date(+now - now.getTimezoneOffset() * 60000);
  $: date = shift.toISOString().split(/[T\.]/)[0];
  $: time = shift.toISOString().split(/[T\.]/)[1];
  let refresh = 0;
  $: label = `${refresh < 240 ? refresh : refresh / 60}${
    refresh < 240 ? "min" : "hours"
  }`;
  $: step = refresh < 60 ? 1 : refresh < 240 ? 10 : 60;
  $: min = refresh < 60 ? 0 : refresh < 240 ? -160 : -780;
  $: max = refresh < 60 ? 240 : refresh < 240 ? 720 : 1464;

  function onDateChange(e: Event & { currentTarget: HTMLInputElement }) {
    if (e.currentTarget.type == "date") {
      const [yyyy, mm, dd] = e.currentTarget.value.split("-");
      now.setFullYear(+yyyy, +mm - 1, +dd);
    } else {
      const [hh, mm, ss] = e.currentTarget.value.split(":");
      now.setHours(+hh, +mm, ss ? +ss : undefined);
    }
    console.log(e.currentTarget.type, e.currentTarget.value, now);
  }

  function syncDate() {
    now = new Date();
    applySettings();
  }

  const pattern = [
    { value: "normal", text: "Normal" },
    { value: "shuffle", text: "Shuffle" },
  ];
  let order = pattern[0].value;
</script>

<main>
  <Container>
    <span slot="title">Settings</span>
    <input type="date" value={date} on:change={onDateChange} />
    <input type="time" value={time} on:change={onDateChange} step="1" />

    <fieldset>
      <legend>Refresh interval: {label}</legend>
      <input type="range" {min} {max} {step} bind:value={refresh} />
    </fieldset>

    <fieldset>
      <legend>Display order</legend>
      <select bind:value={order}>
        {#each pattern as { value, text }}
          <option {value}>{text}</option>
        {/each}
      </select>
    </fieldset>

    <div>
      <button class="contrast" on:click={syncDate}>Sync</button>
      <button on:click={applySettings}>Apply</button>
      <button class="secondary" on:click={initSettings}>Reset</button>
    </div>
  </Container>
</main>

<Snackbar error={snackbar.error} bind:open={snackbar.show}>
  <div>{snackbar.text}</div>
</Snackbar>
