<script lang="ts">
  import { Icon } from "smelte";
  export let icon: string;
  export let title: string;
</script>

<section class="flex justify-between border-gray-500 border-t p-4">
  <span class="text-l flex text-gray-700 font-bold">
    <Icon class="text-l align-text-bottom self-center mr-2">{icon}</Icon>
    {title}:
  </span>
  <span class="align-text-bottom self-center text-gray-800">
    <slot />
  </span>
</section>
