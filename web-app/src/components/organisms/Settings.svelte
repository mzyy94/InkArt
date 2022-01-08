<script lang="ts">
  import { onMount } from "svelte";
  import { Button, DatePicker, Snackbar } from "smelte";
  import api from "../../api";
  import Container from "../templates/Container.svelte";
  import TimeInput from "../atoms/TimeInput.svelte";
  import TimeZoneInput from "../atoms/TimeZoneInput.svelte";

  const fullFormat: Intl.DateTimeFormatOptions = {
    year: "numeric",
    month: "long",
    day: "numeric",
    weekday: "short",
    hour: "numeric",
    minute: "numeric",
    second: "numeric",
    hour12: false,
  };

  async function initSettings() {
    const { time } = await api.config();
    date = new Date(time);
  }

  function applySettings() {
    const defaultDate = Intl.DateTimeFormat("en-US", fullFormat).format(date);
    const modifiedDate = Intl.DateTimeFormat("en-US", {
      timeZone,
      ...fullFormat,
    }).format(date);
    const timeZoneOffset =
      new Date(defaultDate).getTime() - new Date(modifiedDate).getTime();
    const datetime = new Date(date.getTime() + timeZoneOffset);

    api.config({ time: datetime.toJSON() }).then((res) => {
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

  let { timeZone } = new Intl.DateTimeFormat("default", {}).resolvedOptions();
  let date: Date = new Date();

  function onDateChange(e: CustomEvent<Date>) {
    const d = e.detail;
    date.setFullYear(d.getFullYear(), d.getMonth(), d.getDate());
  }

  function syncDate() {
    date = new Date();
  }
</script>

<main>
  <Container>
    <span slot="title">Settings</span><DatePicker
      value={date}
      on:change={onDateChange}
    />
    <TimeInput bind:value={date} />
    <TimeZoneInput bind:timeZone />

    <div class="flex space-x-2 justify-end">
      <Button color="blue" on:click={syncDate}>Sync</Button>
      <Button on:click={applySettings}>Apply</Button>
      <Button color="secondary" on:click={initSettings}>Reset</Button>
    </div>
  </Container>
</main>

<Snackbar color={snackbar.color} bind:value={snackbar.show}>
  <div>{snackbar.text}</div>
</Snackbar>
