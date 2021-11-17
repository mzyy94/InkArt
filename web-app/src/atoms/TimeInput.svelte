<script lang="ts">
  import { onMount } from "svelte";
  import { TextField } from "smelte";

  export let value: Date;
  $: time = value.toLocaleTimeString("en-US", { hour12: false });

  let inputError = "";

  let interval = -1;
  function startCounter() {
    self.clearInterval(interval);
    let lastUpdate = Date.now();
    interval = self.setInterval(() => {
      const now = Date.now();
      value = new Date(value.getTime() + now - lastUpdate);
      lastUpdate = now;
    }, 1000);
  }

  onMount(startCounter);

  function inputTime(e: Event) {
    const input = e.target as HTMLInputElement;
    const pattern = new RegExp(input.pattern);

    if (pattern.test(input.value)) {
      inputError = "";
      const [hh, mm, ss] =
        input.value.split(":")?.map((t) => parseInt(t, 10)) ?? [];
      value.setHours(hh, mm, ss);
      startCounter();
    } else {
      inputError = "Invalid time format.";
    }
  }
</script>

<TextField
  value={time}
  error={inputError}
  on:change={inputTime}
  on:focus={() => clearInterval(interval)}
  append="access_time"
  label="Time"
  pattern="^\d\d:\d\d:\d\d$"
/>
