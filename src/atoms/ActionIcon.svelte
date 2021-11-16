<script lang="ts">
  import { Icon } from "smelte";
  export let icon: string;
  export let callback: Function;
</script>
<Icon on:click={callback()} class="cursor-pointer">{icon}</Icon>