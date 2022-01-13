<script lang="ts">
  import { onMount } from "svelte";
  import { Button, DatePicker, Snackbar, Slider } from "smelte";
  import api from "../../api";
  import Container from "../templates/Container.svelte";
  import TimeInput from "../atoms/TimeInput.svelte";

  async function initSettings() {
    const config = await api.config();
    date = new Date(config.time);
    refresh = config.refresh;
  }

  function applySettings() {
    api.config({ time: date.getTime(), refresh }).then((res) => {
      snackbar.text = `Update settings ${res.ok ? "succeeded" : "failed"}`;
      snackbar.color = res.ok ? "primary" : "error";
      snackbar.show = true;
    });
  }

  onMount(initSettings);

  let snackbar = {
    show: false,
    text: "",
    color: "primary",
  };

  let date: Date = new Date();
  let refresh = 0;
  $: label = `${refresh < 240 ? refresh : refresh / 60}${
    refresh < 240 ? "min" : "hours"
  }`;
  $: step = refresh < 60 ? 1 : refresh < 240 ? 10 : 60;
  $: min = refresh < 60 ? 0 : refresh < 240 ? -160 : -780;
  $: max = refresh < 60 ? 240 : refresh < 240 ? 720 : 1464;

  function onDateChange(e: CustomEvent<Date>) {
    const d = e.detail;
    date.setFullYear(d.getFullYear(), d.getMonth(), d.getDate());
  }

  function syncDate() {
    date = new Date();
    applySettings();
  }
</script>

<main>
  <Container>
    <span slot="title">Settings</span>
    <DatePicker value={date} on:change={onDateChange} />
    <TimeInput bind:value={date} />

    <fieldset class="my-3">
      <p class="text-gray-700">Refresh interval: {label}</p>
      <Slider {min} {max} {step} bind:value={refresh} />
    </fieldset>

    <div class="flex space-x-2 justify-end">
      <Button color="error" on:click={syncDate}>Sync</Button>
      <Button on:click={applySettings}>Apply</Button>
      <Button color="gray" on:click={initSettings}>Reset</Button>
    </div>
  </Container>
</main>

<Snackbar color={snackbar.color} bind:value={snackbar.show}>
  <div>{snackbar.text}</div>
</Snackbar>
